/*
 * File:   main.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Exercise: W06E01 - Digit Display
 * 
 * Description:
 * This program receives characters from serial terminal.
 * If received character is a digit 0-9, it will be displayed on the
 * seven segment display. Otherwise letter E is displayed. Serail terminal
 * also tells, if a character is accepted or rejected.
 * 
 * Created on November 29, 2021, 14:20
 */


#include <avr/io.h>
#include "FreeRTOS.h"
// Copied from course material
#include "clock_config.h"   
#include "task.h"
#include "queue.h"
// Used to send strings to serial terminal
#include <string.h>         

// Setting macro to apply BAUD RATE
// Copied from Microchip's Getting Started with USART
#define USART0_BAUD_RATE(BAUD_RATE) \
((float)(configCPU_CLOCK_HZ * 64 / (16 *(float)BAUD_RATE)) + 0.5)

// QUEUES
// Queue for seven segment numbers
static QueueHandle_t number_queue;
// Queue for USART messages
static QueueHandle_t message_queue;

// Seven sgement digits 0-9, letter E and blank.
const uint8_t digit[] =
{
    0b00111111,     // 0
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,     // 5
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,     // 9
    0b01111001,     // E
    0b00000000      // blank
};

// Function to initialize queues
void queue_init(void)
{
    // Setting max length of both queues
    const uint8_t number_queue_len = 10;
    const uint8_t message_queue_len = 10;
    
    // Create queues, and set their size to size of uint8_t (1 byte)
    number_queue = xQueueCreate(number_queue_len, sizeof(uint8_t));
    message_queue = xQueueCreate(message_queue_len, sizeof(uint8_t));
}

// Copied from Microchip's Getting Started with USART
// Used to read serial terminal commands
uint8_t USART0_read(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm))
    {
        ;
    }
    return USART0.RXDATAL;
}

// Copied from Microchip's Getting Started with USART
// Used to write characters to serial terminal
void USART0_sendChar(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART0.TXDATAL = c;
}

// Copied from Microchip's Getting Started with USART
// Used to write strings to serial terminal
void USART0_sendString(char *str)
{
    for(size_t i = 0; i < strlen(str); i++)
    {
        USART0_sendChar(str[i]);
    }
}

// Function to initialize USART0
void USART0_init(void)
{
    // Setting PA0 as output (RX)
    PORTA.DIRSET = PIN0_bm;
    // Setting PA1 as input (TX)
    PORTA.DIRCLR = PIN1_bm;
    
    //Setting baud rate to 9600 using macro
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    // Enable receiver and transmitter
    USART0.CTRLB |= (USART_RXEN_bm) | (USART_TXEN_bm);
}

// Task method to send messages to serial terminal
void message_send(void* parameter)
{
    // Create variable to hold received message from message_queue
    char rcv_msg;
    // This task will run indefinitely
    for(;;)
    {  
        // Checking if there is a message in the queue
        if (xQueueReceive(message_queue, (void *)&rcv_msg, 0) == pdTRUE) 
        {
            // Cheking if received message is number 0-9
            if(rcv_msg >= '0' && rcv_msg <= '9')
            {
                // Informing a user received message was a digit
                USART0_sendString("You gave a valid digit!\r\n");
            }
            else
            {
                // Sending error message, because message was not a digit
                USART0_sendString("Error! Not a valid digit\r\n");
            }
        }
    }
    // Above loop will not end, but vTaskDelete is used to delete 
    // a finished task.
    vTaskDelete(NULL);
}

// Task method that receives commands from serail terminal
void message_receive(void* parameter)
{   
    // Create variable to hold received character
    char msg;
    // This task will run indefinitely
    for(;;)
    {
        // Reading serial terminal and save received command to variable
        msg = USART0_read();
        // Send message to number queue
        xQueueSend(number_queue, (void *)&msg, 10);
        // Send message to msg queue
        xQueueSend(message_queue, (void *)&msg, 10);
    }
    // Above loop will not end, but vTaskDelete is used to delete 
    // a finished task.
    vTaskDelete(NULL);
}

// Task method which handles displayed numbers
void seven_segment_numbers(void* parameter)
{
    // Set PF5 (on-board LED) as output.
    // Drives the 7-segment grounding transistor.
    PORTF.DIRSET = PIN5_bm;
    // Set PF5 HIGH to ground the 7-segment display
    // (and turn off the on-board LED)
    PORTF.OUTSET = PIN5_bm;
    
    // Set PORTC (seven segment display) as output
    PORTC.DIRSET = 0xFF;
    
    // Create variable to hold received message from number_queue
    char rcv_msg;
    // Array index to set number to seven segment display
    // Default 11 to display blank
    uint8_t index = 11;
    // This task will run indefinitely
    for(;;)
    {
        // Checking if there is a character in the queue
        if (xQueueReceive(number_queue, (void *)&rcv_msg, 0) == pdTRUE) 
        {
            // Switch-case to display number if received character is a number
            // Otherwise displays a letter E
            switch(rcv_msg)
            {
                case '0':
                    index = 0;
                    break;
                
                case '1':
                    index = 1;
                    break;
                
                case '2':
                    index = 2;
                    break;
                
                case '3':
                    index = 3;
                    break;
                
                case '4':
                    index = 4;
                    break;
                
                case '5':
                    index = 5;
                    break;
                
                case '6':
                    index = 6;
                    break;
                
                case '7':
                    index = 7;
                    break;
                
                case '8':
                    index = 8;
                    break;
                
                case '9':
                    index = 9;
                    break;
                // If non of comparison above is true, then display letter E
                default:
                    index = 10;
                    break;
            };
        }
        // Display digit on the seven segment display
        VPORTC.OUT = digit[index];
    }
    // Above loop will not end, but vTaskDelete is used to delete 
    // a finished task.
    vTaskDelete(NULL);
}


int main(void)
{
    // Call queue_init to intialize both queues
    queue_init();
    // Call USART0_init to initialize USART communication
    USART0_init();
    
    // Create a new task for seven_segment_numbers
    xTaskCreate(
        seven_segment_numbers,
        "number",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    );
    // Create a new task for message_send
    xTaskCreate(
        message_send,
        "send",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    );    
    // Create a new task for message_receive
    xTaskCreate(
        message_receive,
        "receive",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    ); 
    // Start the scheduler
    vTaskStartScheduler();
    // Scheduler will not return
    return 0;
}
