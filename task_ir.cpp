#include <Arduino.h>
#include <PrintStream.h>
#include "shares.h"
#include "task_ir.h"
#include <Adafruit_AMG88xx.h> //IR camera code from adafruit




/** @brief 
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_ir (void* p_params)
{
    uint8_t state = 0;                 // State machine takes data or doesn't
    uint8_t counter = 0;
    uint8_t validate = 0;

    for (;;)
    {
        // In state 0 we're waiting for signal from control task to take data 
        if (state == 0)
        {
            if (pixels.get() > 34) //Dummy code. This needs to use the I2C data from the IR camera to find if any pixel is over 34 deg C (approximate skin temp)
            {
                validate++;
                if (validate > 10)
                {
                    cam_share.put(true);
                    DC_share.put(true);
                    state = 1;
                    validate = 0;
                }
            }
        }
        else if (state == 1)
        {
            if (pixels.get() < 34) 
            {
                validate++;
                if (validate > 30)
                {
                    cam_share.put(false);
                    DC_share.put(false);
                    stepper_share.put(true);
                    state = 2;
                    validate = 0;
                }
            }
        }
        else if (state == 2)
        {
            if (ir_share.get());
            {
                state = 0;
                ir_share.put(false);
            }
        }

        vTaskDelay (100);
    }
}


        // // In state 1 data is being taken until the number of points per set
        // // have been put into the queue
        // else if (state == 2)
        // {
        //     // Read one data point and immediately stuff it into the queue
        //     data_queue.put (analogRead (DATA_ACQ_PIN));

        //     // Check if we've taken the required number of data points yet
        //     if (++counter > POINTS_PER_SET)
        //     {
        //         state = 0;
        //     }
        // }