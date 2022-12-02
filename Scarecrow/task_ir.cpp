/** @file   task_ir.cpp
 *  This file contains the IR task, used to detect heat sources that need scaring.
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
#include "task_ir.h"
#include <Adafruit_AMG88xx.h> //IR camera code from adafruit


/** @brief   Task which detects heat sources and communicates to other tasks this info.
 *  @details If a pixel detects > 27C for 1 second, it needs to be scared. 
 *           If no pixel detects > 27C for 3 seconds, nothing needs to be scared.
 *  @param   p_params An unused pointer to nonexistent parameters
 */
void task_ir (void* p_params)
{
    uint8_t state = 0;      // Start in the patrol state
    uint8_t validate = 0;   // counter used to track how long heat is detected

    #define I2C_SDA 21      // Data line for I2C
    #define I2C_SCL 22      // Clock line for I2C

    /** @brief   The I2C (TwoWire) object for the IR sensor.
     *  @details I2CAMG is used to send pixel data from the sensor to the ESP32.
     */
    TwoWire I2CAMG = TwoWire(0);

    /** @brief   object for the Adafruit IR sensor.
     *  @details amg is used to collect pixel data from the IR sensor.
     */
    Adafruit_AMG88xx amg;

    float pixels[AMG88xx_PIXEL_ARRAY_SIZE];          // array of 64 IR sensor pixels


    I2CAMG.begin(I2C_SDA, I2C_SCL, 100000);          // Establish I2C to the ESP32
    Serial.println(F("AMG88xx pixels"));
    bool status;
    // default settings
    status = amg.begin(0x69, &I2CAMG);  //0x69, &I2CAMG
    if (!status) {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }
    delay(100);


    for (;;)
    {
        amg.readPixels(pixels); //IR cam scans regardless of state
        
        // Scarecrow is searching. Stepper motor is on (panning back and forth slowly)
        if (state == 0)
        {
            if (pixels[0] > 27)  //IR sensor detects a pixel > 27C (warm body)
            {
                validate++;
                if (validate > 4)  //Heat detected for 1 second, so assume it is life and needs to be scared.
                {
                    Serial.print("\n1 second of heat > 30C\n");
                    cam_share.put(true);        //Tells cam task to take a picture
                    motors_share.put(false);    //Tells motors task to turn off stepper motor and turn on DC motor
                    state = 1;
                    validate = 0;  //reset counter
                }
            }
        }

        // Scarecrow is scaring. Stepper motor is off
        else if (state == 1)
        {
            if (pixels[0] < 27) //Sensor detects < 27C (No warm body)
            {
                validate++;
                if (validate > 12)  //No heat detected for 3 seconds, so assume life has been scared away
                {
                    //cam_share is put to false inside the cam_task. Should not be done here.
                    Serial.print("3 seconds of no heat > 30C\n");
                    motors_share.put(true);    //Tells motors task to turn on stepper motor and turn off DC motor
                    state = 0;
                    validate = 0;  //reset counter
                }
            }
        }

        vTaskDelay (250);  //Period of 250 ms
    }
}