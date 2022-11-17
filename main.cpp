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


/// A share which holds a counter of how many times a simulated event occurred
Share<bool> begin_recording ("Recording");

/// A queue which holds a bunch of data taken by a measurement task
Queue<uint16_t> data_queue (100, "Data");



/** @brief   Runs once for setting up tasks and functions.
 */
void setup() 
{

  Serial.begin (115200);
  while (!Serial) 
  {
  }
  xTaskCreate (task_ir, "ir", 2048, NULL, 5, NULL);
  xTaskCreate (task_DC, "DC", 2048, NULL, 2, NULL);
  xTaskCreate (task_stepper, "stepper", 2048, NULL, 2, NULL);
  xTaskCreate (task_cam, "cam", 2048, NULL, 2, NULL);
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