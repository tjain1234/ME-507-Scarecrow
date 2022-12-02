/** @file   task_cam.cpp
 *  This file contains the cam task, used to tell the ESP32 CAM to 
 *  take a photo when heat is detected for 1 second.
 * 
 *  @author Ethan Nikcevich
 *  @author Miles Ibarra
 *  @author Tim Jain
 *  
 *  @date   11/27/2022
 *  @copyright 2022 by the authors, released under the MIT License.
 */

#include <Arduino.h>
#include <PrintStream.h>
#include "shares.h"
#include "task_cam.h"
#include <esp_camera.h>

/** @brief   Task which controls when a picture is taken.
 *  @details This task writes a pin connected to an input pin
 *           on the ESP32 CAM to high, so the ESP32 CAM knows to take a photo.
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_cam (void* p_params)
{
    uint8_t state = 0;

    //Pin on ESP32 used to tell Cam to take a picture
    #define cam 18        
    pinMode(cam, OUTPUT);

    for (;;)
    {
        // camera is waiting to be told to take a photo
        if (state == 0)
        {
            if (cam_share.get()) //if cam_share is true, the Cam needs to take a photo
            {
                Serial.print("Cam takes a picture now\n");
                camphoto_share.put(true);   //Tells the Web that a photo was taken
                digitalWrite(cam, HIGH);    //set the pin connected to the cam to high, so the cam knows to take a pic.
                delay (100);
                digitalWrite(cam, LOW); 
                cam_share.put(false);       //don't take more than one picture and wait for next signal from task_ir
            }
        }

        vTaskDelay(500);  //Period of 500 ms
    }
}
