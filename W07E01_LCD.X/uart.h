/* 
 * File:   uart.h
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Created on December 7, 2021, 5:27 PM
 */

#ifndef USART_H
#define	USART_H

// Macro to set baud rate, copied from course materials
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(configCPU_CLOCK_HZ * 64 / (16 * \
(float)BAUD_RATE)) + 0.5)
// Declaring functions
void USART0_sendString(char *str);
void usart0_write(void* param);
void usart0_init(void);

#endif	/* USART_H */