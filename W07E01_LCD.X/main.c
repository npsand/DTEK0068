/*
 * File:   main.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * Exercise: W07E01 - LCD
 * Description:
 * This program display text and values on the LCD display.
 * Values that are on the LCD are from LDR, POT and NTC.
 * LCD backlight is adjustet using LDR value. LCD backlight turns off
 * automatically after 10 seconds if potentiometer value stays exactly same!
 * 
 * Created on December 9, 2021, 4:20 PM
 */


#include <avr/io.h> 
// FreeRTOS
#include "FreeRTOS.h" 
// Including files to use spesific functions and create tasks
#include "adc.h"
#include "uart.h"
#include "dummy.h"
#include "display.h"
#include "backlight.h"

// Initialize TCB3
void TCB3_init (void)
{
    // Load CCMP register with the period and duty cycle of the PWM
    TCB3.CCMP = 0x80FF;
    // Enable TCB3 and Divide CLK_PER by 2
    TCB3.CTRLA |= TCB_ENABLE_bm;
    TCB3.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
    // Enable pin output and configure TCB in 8-bit PWM mode
    TCB3.CTRLB |= TCB_CCMPEN_bm;
    TCB3.CTRLB |= TCB_CNTMODE_PWM8_gc;
}

  
int main(void)
{
    // Create mutex
    mutex = xSemaphoreCreateMutex();
    // Create queue for acd data
    lcd_data_queue = xQueueCreate(1, sizeof(ADC_result_t));
    // Initialize adc
    adc_init();
    // Initialize usart0
    usart0_init();
    // Initialize TCB3
    TCB3_init();
    // Initialize backlight
    backlight_init();
    
    // TASKS
    xTaskCreate( 
        display_task, 
        "display", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY, 
        NULL 
    );
    
    xTaskCreate( 
        lcd_task, 
        "lcd", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY, 
        NULL 
    );   
    
   xTaskCreate( 
        usart0_write, 
        "write", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY, 
        NULL 
    ); 
   
        xTaskCreate( 
        backlight_task, 
        "backlight", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY, 
        NULL 
    );    
       xTaskCreate( 
        dummy_task, 
        "dummy", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        (configMAX_PRIORITIES - 1), // Priority 10, higher than other tasks
        NULL 
    ); 
       
    // Start the scheduler 
    vTaskStartScheduler(); 
    
    // Scheduler will not return 
    return 0; 
}