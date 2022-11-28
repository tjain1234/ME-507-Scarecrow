#include <Arduino.h>
#include <PrintStream.h>
#include "shares.h"
#include "task_motors.h"

#include <Stepper.h>


/** @brief 
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_motors (void* p_params)
{
    uint8_t state = 0;

    //Stepper Motor Setup
    #define STEPS 200
    #define stepper_AIN2 27
    #define stepper_AIN1 26
    #define stepper_BIN1 12
    #define stepper_BIN2 13
    Stepper stepper(STEPS, stepper_AIN2, stepper_AIN1, stepper_BIN1, stepper_BIN2);
    stepper.setSpeed(60);
    int8_t forward_steps = 5;
    int8_t backward_steps = -5;
    uint8_t counter = 0;

    //DC Motor Setup
    #define dc_AIN2 33
    #define dc_AIN1 32
    #define dc_PWMA 25
    pinMode(dc_AIN2, OUTPUT);
    pinMode(dc_AIN1, OUTPUT);
    pinMode(dc_PWMA, OUTPUT);

    for (;;)
    {
        // Stepper motor is on (panning back and forth __slowly__)
        if (state == 0)
        {
            if (motors_share.get()) //if stepper_share is true, the stepper motor needs to pan back and forth, DC motor needs to be off
            {
                //Stepper motor moves forward 16*5=80 steps, then backward 16*-5=-80 steps, then repeats.
                if (counter < 16)
                {
                    counter++;
                    stepper.step(forward_steps);
                }
                else if (counter < 32)
                {
                    counter++;
                    stepper.step(backward_steps);
                }
                else
                {
                    counter = 0;
                }
                
                //DC motor does not move
                digitalWrite(dc_AIN2, LOW);
                digitalWrite(dc_AIN1, LOW); 

            }
            else
            {
                Serial.print("DC motor should be on and Stepper motor should be off");
                state = 1;
            }
        }

        // Stepper motor is not moving and dc motor is on to move arms
        else if (state == 1)
        {
            if (motors_share.get()) //if stepper_share is true, the stepper motor needs to pan back and forth (state 0)
            {
                Serial.print("DC motor should be off and Stepper motor should be on patrol");
                state = 0;
            }
            else
            {
                
                //DC motor spins
                digitalWrite(dc_AIN2, LOW);
                digitalWrite(dc_AIN1, HIGH); 

                //stepper motor does not move
            }
        }

        vTaskDelay (750);
    }
}
