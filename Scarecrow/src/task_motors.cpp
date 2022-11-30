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
    Stepper stepper(STEPS, stepper_AIN1, stepper_AIN2, stepper_BIN1, stepper_BIN2);
    stepper.setSpeed(80);
    int8_t forward_steps = 10;
    int8_t backward_steps = -10;
    uint8_t counter = 0;

    //DC Motor Setup for original PCB layout
    // #define dc_AIN2 33
    // #define dc_AIN1 32
    // #define dc_PWMA 25
    // pinMode(dc_AIN2, OUTPUT);
    // pinMode(dc_AIN1, OUTPUT);
    // pinMode(dc_PWMA, OUTPUT);

    //DC Motor Setup for Breadboard Backup
    #define DCPin 14
    pinMode(DCPin, OUTPUT);


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
                    Serial.print("f");
                }
                else if (counter < 32)
                {
                    counter++;
                    stepper.step(backward_steps);
                    Serial.print("b");
                }
                else
                {
                    counter = 0;
                }
                
                //DC motor does not move (PCB)
                // digitalWrite(dc_AIN2, LOW);
                // digitalWrite(dc_AIN1, LOW); 

                //DC motor does not move (Breadboard)
                digitalWrite(DCPin, LOW);

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
                Serial.print("should write pins to move motor here \n");
                //DC motor spins (PCB)
                // digitalWrite(dc_AIN2, HIGH);
                // digitalWrite(dc_AIN1, LOW); 

                //DC motor spins (Breadboard)
                digitalWrite(DCPin, HIGH);

                //stepper motor does not move
            }
        }

        vTaskDelay (1000);
    }
}
