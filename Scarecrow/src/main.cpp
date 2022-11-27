/** @file   main.cpp
 *  This file creates two tasks called task_Debounce and task_Wave that run simultaneously. 
 *  Each task interacts with its own GPIO pin, both of which are also setup in this file.
 *  @author Ethan Nikcevich
 *  @date   11/9/2022
 */


#include <Arduino.h>
#include <PrintStream.h>
#include "taskshare.h"         // Header for inter-task shared data
#include "taskqueue.h"         // Header for inter-task data queues
#include "shares.h"            // Header for shares used in this project
#include "task_ir.h"
#include "task_cam.h"
#include "task_motors.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

// #define I2C_SDA 21                               // This block is IR cam stuff
// #define I2C_SCL 22
// TwoWire I2CAMG = TwoWire(0);
// Adafruit_AMG88xx amg;
// float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

// /// A share which holds a counter of how many times a simulated event occurred
// Share<bool> begin_recording ("Recording");

// /// A queue which holds a bunch of data taken by a measurement task
// Queue<uint16_t> data_queue (100, "Data");

Share<bool> cam_share ("cam");
Share<bool> motors_share ("motors");

/** @brief   Runs once for setting up tasks and functions.
 */
void setup() 
{

  Serial.begin (115200);
  delay(3000);
  
  while (!Serial) 
  {
  }
  
  // I2CAMG.begin(I2C_SDA, I2C_SCL, 100000);          // This block is IR cam stuff
  // Serial.println(F("AMG88xx pixels"));
  // bool status;
  // // default settings
  // status = amg.begin(0x69, &I2CAMG);  //0x69, &I2CAMG
  // if (!status) {
  //   Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
  //   while (1);
  // }
  // delay(100);


  //ir_share.put(true);
  motors_share.put(true);
  cam_share.put(false);

  xTaskCreate (task_ir, "ir", 2048, NULL, 10, NULL);
  xTaskCreate (task_motors, "stepper", 2048, NULL, 7, NULL);
  xTaskCreate (task_cam, "cam", 2048, NULL, 5, NULL);
}

/** @brief   The Arduino loop function.
 *  @details This function is called periodically by the Arduino system. It
 *           runs as a low priority task. On some microcontrollers it will
 *           crash when FreeRTOS is running, so we usually don't use this
 *           function for anything, instead just having it delay itself. 
 */
void loop (void)
{
  vTaskDelay(1000);
}