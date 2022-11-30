#include <Arduino.h>
#include <PrintStream.h>
#include "shares.h"
#include "task_cam.h"
#include <esp_camera.h>

/** @brief 
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_cam (void* p_params)
{
    uint8_t state = 0;

    #define cam 18        
    pinMode(cam, OUTPUT);

    
    //Define any variables and pins needed to run the esp32cam here.

    for (;;)
    {
        // camera is waiting to be told to take a photo
        if (state == 0)
        {
            if (cam_share.get()) //if cam_share is true, the esp32cam needs to take a picture
            {
                Serial.print("Cam should take a picture now");
                digitalWrite(cam, HIGH);    //set the pin connected to the cam to high, so the cam knows to take a pic.
                delay (100);
                digitalWrite(cam, LOW); 
                cam_share.put(false);  //don't take more than one picture and wait for next signal from task_ir
            }
        }

        vTaskDelay(500);
    }
}
