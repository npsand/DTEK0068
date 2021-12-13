/*
 * File:   uart.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Prints text and values to the serial terminal via USART0
 * 
 * Created on December 7, 2021, 5:27 PM
 */

#include <avr/io.h>
#include <stdio.h>
// FreeRTOS
#include "FreeRTOS.h" 

#include "uart.h" // To get E.g. baud rate
#include "adc.h" // To get ADC readings


// Copied from documentation
void usart0_send_char(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART0.TXDATAL = c;
}

// Copied from documentation
int usart0_print_char(char c, FILE *stream)
{ 
    usart0_send_char(c);
    return 0; ; 
}

// Copied from documentation
static FILE USART_stream = FDEV_SETUP_STREAM(usart0_print_char, NULL, _FDEV_SETUP_WRITE);

void usart0_init(void)
{
    // Setting PA0 as output (RX)
    PORTA.DIRSET = PIN0_bm;
    // Setting PA1 as input (TX)
    PORTA.DIRCLR = PIN1_bm;
    
    //Setting baud rate to 9600 using macro
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    // Enable transmitter
    USART0.CTRLB |= (USART_TXEN_bm);
    // Setting standard output
    stdout = &USART_stream;
}

void usart0_write(void* param)
{
    ADC_result_t output_buffer; // Store value from output queue
    
    for(;;)
    {       
        // Take mutex, get ADC values and free mutex
        xSemaphoreTake(mutex, 100);
        output_buffer = adc_read();
        xSemaphoreGive(mutex);
        // Print to serail terminal
        printf("LDR: %d\tNTC: %d\tPOT: %d\r\n", output_buffer.ldr, output_buffer.ntc, output_buffer.pot);
        // 1s delay
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    // This task will run infinitely
    vTaskDelete(NULL);
}