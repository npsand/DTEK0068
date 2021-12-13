/* 
 * File:   display.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Display text and variables on the LCD
 * 
 * Created on December 9, 2021, 4:20 PM
 */

#include <avr/io.h> 
#include <stdio.h>  // To use sprintf function
#include <string.h> // To use strlen function
// FreeRTOS
#include "FreeRTOS.h" 
#include "timers.h" // To use timers

#include "lcd.h" // To use lcd fucntions
#include "adc.h" // To access ADC values
#include "display.h" // To access variables

// Scrolling text
const char g_scrolling_text[] = "DTEK0068 Embedded Microprocessor Systems";

void display_task(void *param)
{
    // Declare variable for ADC readings
    ADC_result_t adc_results;
    
    for(;;)
    {
        // Take mutex to access ADC readings
        xSemaphoreTake(mutex, 100);
        adc_results = adc_read();
        xQueueOverwrite(lcd_data_queue, &adc_results);
        // Free mutex
        xSemaphoreGive(mutex);
        // Small delay :)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

// Callback function for display timer
// Increases display_mode until 3 and then resets it
void display_callback()
{
    if(display_mode == 2)
    {
        display_mode = 0;
    }
    else
    {
        display_mode++;
    }
}
// Callback function for scroll_timer
// Changes scroll direction when needed
void scroll_callback()
{
    if(leftmost_char == strlen(g_scrolling_text)-16)
    {
        direction = 1;
    }
    else if(leftmost_char == 0)
    {
        direction = 0;
    }
    if(direction == 0)
    {
        leftmost_char++;
    }
    else{
        leftmost_char--;
    }
}

void lcd_task(void *param)
{
    // Init the lcd
    lcd_init();
    // Set scroll direction
    direction = 0;
    // Sets the leftmost char
    leftmost_char = 0;
    TimerHandle_t display_timer = xTimerCreate
          ( "Timer",
            pdMS_TO_TICKS(660), // 660ms per text
            pdTRUE, // Restart timer automatically when expired
            ( void * ) 0,
            display_callback);
    
        TimerHandle_t scroll_timer = xTimerCreate
          ("Scroll",
            pdMS_TO_TICKS(200), // 1000/5 ~ 5 characters per second
            pdTRUE, // Restart timer automatically when expired
            ( void * ) 1,
            scroll_callback);
    // Start both timers
    xTimerStart(scroll_timer, 10);
    xTimerStart(display_timer, 10);
    
    // Reserve memory for ADC readings
    char adc_val[10];
    
    // Declare variable for ADC readings
    ADC_result_t adc_results;
    
    for(;;)
    {
        // Check if queue receives data
        if(xQueueReceive(lcd_data_queue, &adc_results, 100) == pdTRUE)
        {
            // Print ADR values to LCD regarding display_mode variable
            // which is controlled by timer
            switch(display_mode)
            {
                case 0:
                    sprintf(adc_val, "LDR value: %d", adc_results.ldr);
                    lcd_clear();
                    lcd_write(adc_val);
                    break;
                case 1:
                    sprintf(adc_val, "NTC value: %d", adc_results.ntc);
                    lcd_clear();
                    lcd_write(adc_val);
                    break;
                case 2:
                    sprintf(adc_val, "POT value: %d", adc_results.pot);
                    lcd_clear();
                    lcd_write(adc_val);
                    break;
                default:
                    break;
            }
        }
        // Change LCD line
        lcd_cursor_set(1, 0);
        // Sets scrolling text to the right position
        strncpy(display_scroll_text, g_scrolling_text+leftmost_char, 16);
        // Display the text
        lcd_write(display_scroll_text);
    }
    // This task never ends
    vTaskDelete(NULL);
}