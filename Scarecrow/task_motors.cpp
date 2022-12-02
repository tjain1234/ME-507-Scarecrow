/** @file   task_motors.cpp
 *  This file contains the motors task, used to control the 
 *  stepper and DC motors responsible for patrol and scaring. 
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
#include "task_motors.h"

#include <Stepper.h>


/** @brief   Task which controls the stepper and DC motor
 *  @details If scaring, stepper is off while DC is on to move arms.
 *           If patrolling, stepper is on to swivel IR sensor & cam while DC is off.
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_motors (void* p_params)
{
    uint8_t state = 0;

    //Stepper Motor Setup
    //Number of steps in one rotation of the stepper
    #define STEPS 200
    //4 pins on ESP32 board used for stepper control
    #define stepper_AIN2 27
    #define stepper_AIN1 26
    #define stepper_BIN1 12
    #define stepper_BIN2 13

    /** @brief   object stepper of Stepper class for stepper motor control.
     *  @param   Steps Number of steps in one rotation of the stepper
     *  @param   stepper_AIN1 AIN1 pin
     *  @param   stepper_AIN2 AIN2 pin
     *  @param   stepper_AIN1 BIN1 pin
     *  @param   stepper_AIN2 BIN2 pin
     */
    Stepper stepper(STEPS, stepper_AIN1, stepper_AIN2, stepper_BIN1, stepper_BIN2);

    stepper.setSpeed(80);       //stepper speed
    int8_t forward_steps = 5;   //takes 5 steps per forward patrol cycle
    int8_t backward_steps = -5; //takes 5 steps per backward patrol cycle
    uint8_t counter = 0;        //counter for switching between forward and backward cycles

    //DC Motor Setup for PCB
    //
    // #define dc_AIN2 33
    // #define dc_AIN1 32
    // #define dc_PWMA 25
    // pinMode(dc_AIN2, OUTPUT);
    // pinMode(dc_AIN1, OUTPUT);
    // pinMode(dc_PWMA, OUTPUT);

    //DC motor setup for backup due to fried motor driver chips.
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
                //
                // digitalWrite(dc_AIN2, LOW);
                // digitalWrite(dc_AIN1, LOW); 

                //DC motor does not move (backup due to fried motor driver chips)
                digitalWrite(DCPin, LOW);

            }
            else
            {
                Serial.print("DC motor should be on\n");
                Serial.print("Stepper motor should be off\n");
                state = 1;
            }
        }

        // Stepper motor is not moving and dc motor is on to move arms
        else if (state == 1)
        {
            if (motors_share.get()) //if stepper_share is true, the stepper motor needs to pan back and forth (state 0)
            {
                Serial.print("DC motor should be off\n");
                Serial.print("Stepper motor should be on patrol\n");
                state = 0;
            }
            else
            {
                //DC motor spins (PCB)
                //
                // digitalWrite(dc_AIN2, HIGH);
                // digitalWrite(dc_AIN1, LOW); 

                //DC motor spins (backup due to fried motor driver chips)
                digitalWrite(DCPin, HIGH);

                //stepper motor does not move
            }
        }

        vTaskDelay (1000);  //Period of 1000 ms
    }
}
