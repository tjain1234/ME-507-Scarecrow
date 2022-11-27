#include <Arduino.h>
#include <PrintStream.h>
#include "shares.h"
#include "task_motors.h"

/** @brief 
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_motors (void* p_params)
{
    uint8_t state = 0;

    //Define any variables and pins needed to run the motor here.

    for (;;)
    {
        // Stepper motor is on (panning back and forth __slowly__)
        if (state == 0)
        {
            if (motors_share.get()) //if stepper_share is true, the stepper motor needs to pan back and forth, DC motor needs to be off
            {

                //
                //code here to turn off DC motor
                //code here on making the stepper move back and forth
                //

            }
            else                     //stepper motor needs to hold its position and DC motor needs to turn on(state 1)
            {
                state = 1;
            }
        }

        // Stepper motor is not moving and dc motor is on to move arms
        else if (state == 1)
        {
            if (motors_share.get()) //if stepper_share is true, the stepper motor needs to pan back and forth (state 0)
            {
                state = 0;
            }
            else
            {
                
                //
                //code here to turn on DC motor
                //code here to hold the stepper in position
                //

            }
        }

        vTaskDelay (20);
    }
}
