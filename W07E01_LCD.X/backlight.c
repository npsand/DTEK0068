/* 
 * File:   backlight.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Controls backlight of the LCD screen
 * 
 * Created on December 9, 2021, 4:20 PM
 */


#include <avr/io.h>
// FreeRTOS
#include "FreeRTOS.h"
#include "timers.h" // To use timers
// Needs to read POT adc value
#include "adc.h"

// Flag to check if backlight is on
uint8_t g_backlight_on = 1;

// Callback function for the timer
void timeout_callback()
{
    // Set flag to zero (backlight off)
    g_backlight_on = 0;
    // Sets backlight off
    TCB3.CCMP = 0;
}

void backlight_task(void *param)
{
    // Declare variable for adc results
    ADC_result_t adc_result;
    // Potentiometer value on last iteration
    uint16_t pot_last = 0;
    // Declare timer
    TimerHandle_t timeout = xTimerCreate
      ( "timeout",
        pdMS_TO_TICKS(10000), // Keep baclight on for 10 seconds
        pdFALSE, // Disable auto start after expire
        ( void * ) 4,
        timeout_callback);

    for(;;)
    {
        // Take mutex to read ADC
        xSemaphoreTake(mutex, 100);
        // Save ADC value
        adc_result = adc_read();
        // Free mutex
        xSemaphoreGive(mutex);
        // Check ig backlight is on
        if(g_backlight_on == 1)
        {
            // Dim backlight regarding to the LDR value
            // multiply by value 60 seems good
            TCB3.CCMP = adc_result.ldr * 60;
        }
        // Check if duty cycle is zero and pot value is same as last
        // iteration
        if(TCB3.CCMP == 0 && pot_last != adc_result.pot)
        {
            // Set backlight flag to 1, because backlight is on
            g_backlight_on = 1;
        }
        // Check if pot value is same as last iteration
        if(pot_last == adc_result.pot)
        {
            // Start the timer if it is not started yet
            if(xTimerIsTimerActive(timeout) == pdFALSE)
            {
                xTimerStart(timeout, 0);
            }
        }
        else{
            // Stop the timer if it is started
            if(xTimerIsTimerActive(timeout) == pdTRUE)
            {
                xTimerStop(timeout,0);
            }
            // Save the pot value for the next iteration
            pot_last = adc_result.pot;
        }
    }
    // This task run infinitely
    vTaskDelete(NULL);
}
void backlight_init(void)
{   
    // Set PB5 as output
    PORTB.DIRSET = PIN5_bm;
    // Set PB5 HIGH
    PORTB.OUTSET = PIN5_bm;
}