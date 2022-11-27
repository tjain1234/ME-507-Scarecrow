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
    uint8_t state = 0;
    // uint8_t counter = 0;
    uint8_t validate = 0;



    #define I2C_SDA 21                               // This block is IR cam stuff
    #define I2C_SCL 22
    TwoWire I2CAMG = TwoWire(0);
    Adafruit_AMG88xx amg;
    float pixels[AMG88xx_PIXEL_ARRAY_SIZE];


    I2CAMG.begin(I2C_SDA, I2C_SCL, 100000);          // This block is IR cam stuff
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

        // Scarecrow is searching. Stepper motor is on (panning back and forth __slowly__)
        if (state == 0)
        {
            if (pixels[0] > 30) //This needs to use the I2C data from the IR camera to find if any pixel is over 30 deg C (approximate skin temp)
            {
                validate++;
                if (validate > 10)  //Heat detected for 1 second, so assume it is life and needs to be scared.
                {
                    cam_share.put(true);        //Tells cam task to take a picture
                    motors_share.put(false);    //Tells motors task to turn off stepper motor and turn on DC motor
                    state = 1;
                    validate = 0;
                }
            }
        }

        // Scarecrow is scaring. Stepper motor is off
        else if (state == 1)
        {
            if (pixels[0] < 30) 
            {
                validate++;
                if (validate > 30)  //No heat detected for 3 seconds, so assume life has been scared
                {
                    //cam_share is put to false inside the cam_task. Should not be done here.
                    motors_share.put(true);    //Tells motors task to turn on stepper motor and turn off DC motor
                    state = 0;
                    validate = 0;
                }
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