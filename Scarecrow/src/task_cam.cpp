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

    //Define any variables and pins needed to run the esp32cam here.

    for (;;)
    {
        // camera is waiting to be told to take a photo
        if (state == 0)
        {
            if (cam_share.get()) //if cam_share is true, the esp32cam needs to take a picture
            {
                Serial.print("Cam should take a picture now");
                //
                //code here on taking the picture and storing it locally to a micro sd
                //

                cam_share.put(false);  //don't take more than one picture and wait for next signal from task_ir
            }
        }

        vTaskDelay (50);
    }
}
