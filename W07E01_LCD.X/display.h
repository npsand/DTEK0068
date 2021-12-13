/* 
 * File:   display.h
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Created on December 9, 2021, 4:20 PM
 */

#ifndef DISPLAY_H
#define	DISPLAY_H

// Declare functions
void display_task(void *param);
void lcd_task(void *param);
// Declare variables
char display_scroll_text[16];
uint8_t display_mode;
uint8_t leftmost_char;
uint8_t direction;
QueueHandle_t lcd_data_queue;

#endif	/* DISPLAY_H */
