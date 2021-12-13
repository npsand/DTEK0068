/* 
 * File:   dummy.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Blink built in led if ntc > pot value 
 * 
 * Created on December 9, 2021, 4:20 PM
 */


#include <avr/io.h> 
// FreeRTOS
#include "FreeRTOS.h" 

#include "adc.h" // To get POT and NTC values

void dummy_task(void *param)
{
    // Set PF5 as output
    PORTF.DIRSET = PIN5_bm;

    for(;;)
    {
        // Take mutex, get adc values and free mutex...
        xSemaphoreTake(mutex, 100);
        ADC_result_t adc_result = adc_read();
        xSemaphoreGive(mutex);
        
        if(adc_result.ntc > adc_result.pot)
        {
            // Toggle PF5
            PORTF.OUTTGL = PIN5_bm;
        }
        else
        {   // SET PF5 low
            PORTF.OUTSET = PIN5_bm;
        }
        // wait 100ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    // This task runs infinitely
    vTaskDelete(NULL);
}