/** @file   main.cpp
 *  This file sets up the web server, and creates the task structure used to control the scarecrow.
 *  
 *  @author Ethan Nikcevich
 *  @author Miles Ibarra
 *  @author Tim Jain
 *  @author JR Ridgely
 *  @author A. Sinha
 *  
 *  @date   2022-Mar-28 Original web stuff by Sinha
 *  @date   2022-Nov-04 Modified for ME507 use by Ridgely
 *  @date   11/27/2022 Further modified for Scarecrow use by Nikcevich
 *  @copyright 2022 by the authors, released under the MIT License.
 */


#include <Arduino.h>
#include <PrintStream.h>
#include "taskshare.h"         // Header for inter-task shared data
#include "taskqueue.h"         // Header for inter-task data queues
#include "shares.h"            // Header for shares used in this project
#include "task_ir.h"           // Header for IR task
#include "task_cam.h"          // Header for cam task
#include "task_motors.h"       // Header for motors task
#include <Stepper.h>           // Header for Arduino stepper driver

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>  // Header for Adafruit IR sensor

#include <esp_camera.h>        // Header for Espressif ESP Cam

/////////////////
/////////////////

#include <WiFi.h>
#include <WebServer.h>

// #define USE_LAN to have the ESP32 join an existing Local Area Network or 
// #undef USE_LAN to have the ESP32 act as an access point, forming its own LAN
#undef USE_LAN

// If joining an existing LAN, get certifications from a header file which you
// should NOT push to a public repository of any kind
#ifdef USE_LAN
#include "mycerts.h"       // For access to your WiFi network; see setup_wifi()

// If the ESP32 creates its own access point, put the credentials and network
// parameters here; do not use any personally identifying or sensitive data
#else
const char* ssid = "Scary";   // SSID, network name seen on LAN lists
const char* password = "scarecrow";   // ESP32 WiFi password (min. 8 characters)

/* Put IP Address details */
IPAddress local_ip (192, 168, 5, 1); // Address of ESP32 on its own network
IPAddress gateway (192, 168, 5, 1);  // The ESP32 acts as its own gateway
IPAddress subnet (255, 255, 255, 0); // Network mask; just leave this as is
#endif


/// The pin connected to the DC motor controlled through the Web interface
const uint8_t DCPin = 14;


/** @brief   The web server object for this project.
 *  @details This server is responsible for responding to HTTP requests from
 *           other computers, replying with useful information.
 *
 *           It's kind of clumsy to have this object as a global, but that's
 *           the way Arduino keeps things simple to program, without the user
 *           having to write custom classes or other intermediate-level 
 *           structures. 
*/
WebServer server (80);


/** @brief   Get the WiFi running so we can serve some web pages.
 */
void setup_wifi (void)
{
#ifdef USE_LAN                           // If connecting to an existing LAN
    Serial << "Connecting to " << ssid;

    // The SSID and password should be kept secret in @c mycerts.h.
    // This file should contain the two lines,
    //   const char* ssid = "YourWiFiNetworkName";
    //   const char* password = "YourWiFiPassword";
    WiFi.begin (ssid, password);

    while (WiFi.status() != WL_CONNECTED) 
    {
        vTaskDelay (1000);
        Serial.print (".");
    }

    Serial << "connected at IP address " << WiFi.localIP () << endl;

#else                                   // If the ESP32 makes its own LAN
    Serial << "Setting up WiFi access point...";
    WiFi.mode (WIFI_AP);
    WiFi.softAPConfig (local_ip, gateway, subnet);
    WiFi.softAP (ssid, password);
    Serial << "done." << endl;
#endif
}


/** @brief   Put a web page header into an HTML string. 
 *  @details This header may be modified if the developer wants some actual
 *           @a style for her or his web page. It is intended to be a common
 *           header (and style) for each of the pages served by this server.
 *  @param   a_string A reference to a string to which the header is added; the
 *           string must have been created in each function that calls this one
 *  @param   page_title The title of the page
*/
void HTML_header (String& a_string, const char* page_title)
{
    a_string += "<!DOCTYPE html> <html>\n";
    a_string += "<head><meta name=\"viewport\" content=\"width=device-width,";
    a_string += " initial-scale=1.0, user-scalable=no\">\n<title> ";
    a_string += page_title;
    a_string += "</title>\n";
    a_string += "<style>html { font-family: Helvetica; display: inline-block;";
    a_string += " margin: 0px auto; text-align: center;}\n";
    a_string += "body{margin-top: 50px;} h1 {color: #4444AA;margin: 50px auto 30px;}\n";
    a_string += "p {font-size: 24px;color: #222222;margin-bottom: 10px;}\n";
    a_string += "</style>\n</head>\n";
}


/** @brief   Callback function that responds to HTTP requests without a subpage
 *           name.
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           (the insecure Web port) with a request for the main web page, this
 *           callback function is run. It sends the main web page's text to the
 *           requesting machine.
 */
void handle_DocumentRoot ()
{
    Serial << "HTTP request from client #" << server.client () << endl;

    String a_str;
    HTML_header (a_str, "Scarecrow Web Server");
    a_str += "<body>\n<div id=\"webpage\">\n";
    a_str += "<h1>Scarecrow Control Center</h1>\n";
    a_str += "<p><p> <a href=\"/toggle\">Wave Hello</a>\n";
    a_str += "<p><p> <a href=\"/cam\">See when a photo is taken</a>\n";
    a_str += "</div>\n</body>\n</html>\n";

    server.send (200, "text/html", a_str); 
}


/** @brief   Respond to a request for an HTTP page that doesn't exist.
 *  @details This function produces the Error 404, Page Not Found error. 
 */
void handle_NotFound (void)
{
    server.send (404, "text/plain", "Not found");
}

/**
 * @brief   Tell the user whether a photo has been taken since last checked.
 */
void handle_camprint (void)
{
  if(camphoto_share.get())
  {
    server.send (200, "text/plain", "Photo taken");
    camphoto_share.put(false);
  }
  else 
  {
    server.send (200, "text/plain", "Photo not taken");
  }
}


/** @brief   Toggle the DC motor to wave the arms when called by the web server.
 */
void handle_Toggle_DC (void)
{
    digitalWrite (DCPin, HIGH);  //Turn on the motor
    delay(2000);                 //Wave for 2 seconds
    digitalWrite (DCPin, LOW);   //Turn off the motor

    String toggle_page = "<!DOCTYPE html> <html> <head>\n";
    toggle_page += "<meta http-equiv=\"refresh\" content=\"1; url='/'\" />\n";
    toggle_page += "</head> <body> <p> <a href='/'>Back to main page</a></p>";
    toggle_page += "</body> </html>";

    server.send (200, "text/html", toggle_page); 
}



/** @brief   Task which sets up and runs a web server.
 *  @details After setup, function @c handleClient() must be run periodically
 *           to check for page requests from web clients. One could run this
 *           task as the lowest priority task with a short or no delay, as there
 *           generally isn't much rush in replying to web queries.
 *  @param   p_params Pointer to unused parameters
 */
void task_webserver (void* p_params)
{
    // The server has been created statically when the program was started and
    // is accessed as a global object because not only this function but also
    // the page handling functions referenced below need access to the server
    server.on ("/", handle_DocumentRoot);
    server.on ("/toggle", handle_Toggle_DC);
    server.on ("/cam", handle_camprint);
    server.onNotFound (handle_NotFound);

    // Get the web server running
    server.begin ();
    Serial.println ("HTTP server started");

    for (;;)
    {
        // The web server must be periodically run to watch for page requests
        server.handleClient ();
        vTaskDelay (500);  //Period of 500 ms
    }
}


////////////
////////////

// Shares used for task communication
Share<bool> camphoto_share ("camphoto");
Share<bool> cam_share ("cam");
Share<bool> motors_share ("motors");

/** @brief   Runs once for setting up tasks and functions.
 */
void setup() 
{

  Serial.begin (115200);
  delay (100);
  while (!Serial) { }                   // Wait for serial port to be working
  delay (1000);

  // Call function which gets the WiFi working
  setup_wifi ();

  // Set up the pin for the DC motor on the ESP32 board
  pinMode (DCPin, OUTPUT);
  digitalWrite (DCPin, LOW);

  // Create the tasks
  // Task which runs the web server. It runs at a low priority
  xTaskCreate (task_webserver, "Web Server", 8192, NULL, 2, NULL);
  // Task which runs the IR sensor. High priority
  xTaskCreate (task_ir, "ir", 2048, NULL, 10, NULL);
  // Task which controls the motors
  xTaskCreate (task_motors, "stepper", 2048, NULL, 7, NULL);
  // Task which communicates with the ESP32 CAM
  xTaskCreate (task_cam, "cam", 4096, NULL, 5, NULL);

  //Initial values of shares for task communication
  motors_share.put(true);
  cam_share.put(false);
  camphoto_share.put(false);

}

/** @brief   The Arduino loop function.
 *  @details This function is called periodically by the Arduino system. It
 *           runs as a low priority task. On some microcontrollers it will
 *           crash when FreeRTOS is running, so we usually don't use this
 *           function for anything, instead just having it delay itself. 
 */
void loop (void)
{
  vTaskDelay (1000);
}