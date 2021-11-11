/*
 * File:   main.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Exercise: W03E01 - BombV2
 * 
 * Description:
 * This code simulates "bomb" using a seven-segment display. 
 * The code displays a countdown from 9 to 0 on the display 
 * without using delay. 
 * If an interrupt occurs on PORTA countdown will be halted. When 
 * reaching number 0 start blinking zero on the display.
 * 
 * Note:
 * I use W02E01 wiring for seven segment display.
 * 
 * Created on November 9, 2021, 12:02 PM
 */

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Digits 0 to 9 for seven segment display
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
    0b01101111      // 9
};

// Global variable which defines if the timer is running
// Value 0 when timer is stopped, and other values when timer is running
volatile uint8_t g_running = 1;
// Global variable which is tracking full seconds from clockticks
// Changes 0 to 1 when second is passed
volatile uint16_t g_clockticks = 0;

/*
* This function is copied and modified from technical brief TB3213
*/
void rtc_init(void)
{
    uint8_t temp;
    // Disable oscillator
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_ENABLE_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    // Wait for the clock to be released (0 = unstable, unused)
    while (CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm);

    // Select external crystal (SEL = 0)
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_SEL_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);

    // Enable oscillator
    temp = CLKCTRL.XOSC32KCTRLA;
    temp |= CLKCTRL_ENABLE_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    // Wait for the clock to stabilize
    while (RTC.STATUS > 0);

    // Configure RTC module
    // Select 32.768 kHz external oscillator
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;
    // Enable Periodic Interrupt
    RTC.PITINTCTRL = RTC_PI_bm;
    // Set period to 4096 cycles (1/8 second) and enable PIT function
    RTC.PITCTRLA = RTC_PERIOD_CYC4096_gc | RTC_PITEN_bm;
}

int main(void)
{
    // Set pin PA4 (red wire) as input
    PORTA.DIRCLR = PIN4_bm;

    // Enable internal pull-up resistor for PA4 to make PA4 HIGH 
    // when the red wire is removed
    // Trigger interrupts on rising edge
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;

    // Set PORTC (seven-segment display) as output
    PORTC.DIRSET = 0xFF;
    
    // Set pin PF5 (on board LED) as output
    PORTF.DIRSET = PIN5_bm;
    // Set PF5 to LOW, to shut down the LED and power on 
    // the seven-segment display
    PORTF.OUTSET = PIN5_bm;
    
    // Countdown timer value, starts from number 9
    // value 10 because of the first decrement
    uint8_t countdown = 10;
    
    // Set IDLE sleep mode
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc);

    // Initialize RTC timer's PIT interrupt function
    rtc_init();
    
    // Enable interrupts
    sei();
    
    while (1)
    {    
        // Checks if full second is passed
        if(g_clockticks > 0)
        {
            // Set variable g_clockticks to 0 to wait another full second
            g_clockticks = 0;
            
            // Checks if the countdown should be running
            if(g_running)
            {             
                // Checks if countdown has reachen zero
                if (countdown == 0)
                {
                    // Stop running the countdown
                    g_running = 0;
                }
                else
                {
                    // Decrease number of the countdown 
                    // on the seven-segment display
                    countdown--;
                }
                // Display number on the seven-segment display
                PORTC.OUT = digit[countdown];
            }
            // Triggers "explosion"  
            else if (countdown == 0)
            {
                // Blink on board LED and zero indefinitely
                PORTF.OUTTGL = PIN5_bm;
            }
            // If the red wire is "cutted"
            else
            {
                // Shows number on the seven-segment display
                // and halt the program
                PORTC.OUT = digit[countdown];
            }
        }
        // Re-enter sleep mode after every interrupt wake-up
        sleep_mode();
    }
}


ISR(RTC_PIT_vect)
{
    // Clear interrupt flag
    RTC.PITINTFLAGS = RTC_PI_bm;
    // Initialize variable to count PITs
    static uint8_t counter = 0;
    // Check if PIT count reaches 8 (full second has passed)
    if(counter >= 8)
    {
        // Set global variable g_clockticks to 1
        // to indicate that full second has passed
        g_clockticks = 1;
        // reset the PIT counter
        counter = 0;
    }
    // Increase PIT count
    counter++;
}

ISR(PORTA_PORT_vect)
{
    // Clear specific interrupt flag
    VPORTA.INTFLAGS = PIN4_bm;
    // Set running to 0 to indicate that countdown is over
    g_running = 0;
}