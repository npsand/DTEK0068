/*
 * File:   main.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Exercise: W04E01 - Dino player
 * 
 * Description:
 * This program plays Google Chrome T-REX runner game 
 * using servo, LDR and TRIMPOT. LDR triggers servo when 
 * cacti (brighter than background) is detected. Potentiometer is used to 
 * set threshold value. Threshold value is displayed on the seven segment 
 * display. When LDR readed value is less than or equal to 
 * threshold, servo operates.
 * 
 * 
 * Note:
 * I use W02E01 wiring for the seven segment display.
 * 
 * There is two versions of the T-REX runner game, one with the dark background 
 * and one with the light background. This program is tested for dark 
 * background version of the game.
 * 
 * 
 * Created on November 17, 2021, 14:20
 */
#include <avr/io.h>
#include <avr/cpufunc.h>        // for ccp_write_io()
#include <avr/interrupt.h>

// MACROS FOR DRIVING THE SERVO
#define SERVO_PWM_PERIOD (0x1046)
#define SERVO_PWM_DUTY_NEUTRAL (0x0138)
#define SERVO_PWM_DUTY_MAX  (0x00D0) //0x01A0

// Number 0-10 for the seven segment display. A is signifying number 10.
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
    0b01110111      // A
    
};

// Global variable that checks if servo is returning to neutral position
volatile uint8_t g_neutral = 0;
// Global variable which tells if servo should move to 45 degree 
// position to press spacebar
volatile uint8_t g_press = 0;


/*
 * This function is copied and modified from technical brief TB3213.
 * Initialize RTC to use external crystal.
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
    // Wait for the clock to stabilise
    while (RTC.STATUS > 0);

    // Configure RTC module
    // Select 32.768 kHz external oscillator
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;
    // Enable OVF interrupt
    RTC.INTCTRL |= RTC_OVF_bm; 
    // Enable RTC
    RTC.CTRLA = RTC_RTCEN_bm; 
}

uint16_t ldr_read(void)
{
    // Clear REFSEL
    ADC0.CTRLC &= ~ADC_REFSEL_VDDREF_gc;
    // Set voltagerefrence to internal (2.5v)
    ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;
    // Change MUXPOS to AIN8 to read LDR (PE0)
    ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
    // Start conversion
    ADC0.COMMAND = ADC_STCONV_bm;
    // Wait for ADC0.INTFLAGS to get set
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    // Return read value
    return ADC0.RES;
}

uint16_t trimpot_read(void)
{
    // Clear REFSEL
    ADC0.CTRLC &= ~ADC_REFSEL_VDDREF_gc;
    // Set voltagerefrence to VDD (5v)
    ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;
    // Change MUXPOS to AIN14 to read TRIMPOT (PF4)
    ADC0.MUXPOS = ADC_MUXPOS_AIN14_gc;
    // Start conversion
    ADC0.COMMAND = ADC_STCONV_bm;
    // Wait for ADC0.INTFLAGS to get set
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    // Return read value
    return ADC0.RES;
}


int main(void)
{
    // Set entire PORTC (7-segment LED display) as output
    PORTC.DIRSET = 0xFF;
    
    // SERVO
    // Route TCA0 PWM waveform to PORTB
    PORTMUX.TCAROUTEA |= PORTMUX_TCA0_PORTB_gc;
    // Set 0-WO2 (PB2) as digital output
    PORTB.DIRSET = PIN2_bm;
    // Set TCA0 prescaler value to 16 (~208 kHz)
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;
    // Set single-slop PWM generation mode
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    // Using double-buffering, set PWM period (20 ms)
    TCA0.SINGLE.PERBUF = SERVO_PWM_PERIOD;
    // Set initial servo arm position as neutral (0 deg)
    TCA0.SINGLE.CMP2BUF = SERVO_PWM_DUTY_NEUTRAL;
    // Enable Compare Channel 2
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
    // Enable TCA0 peripheral
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    
    
    
    // Set PF5 (on-board LED) as output.
    // Drives the 7-segment grounding transistor.
    PORTF.DIRSET = PIN5_bm;

    // Set PF5 HIGH to ground the 7-segment display
    // (and turn off the on-board LED)
    PORTF.OUTSET = PIN5_bm;
    
    // LDR
    // Set PE0 (AIN8) as input 
    PORTE.DIRCLR = PIN0_bm;
    // Disable input buffer
    PORTE.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // Set prescaler of 16
    ADC0.CTRLC |= ADC_PRESC_DIV16_gc;
    // Enable ADC
    ADC0.CTRLA |= ADC_ENABLE_bm;
    // Set internal reference voltage to 2.5V
    VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc;
    //ADC0.CTRLA = VREF_ADC0REFSEL_2V5_gc;
    
    // POTENTIOMETER
    // Set PF4 (AIN14) as input 
    PORTF.DIRCLR = PIN4_bm;
    // Disable input buffer
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
    
    // Initialise RTC
    rtc_init();

    // Enable interrupts
    sei();

    while (1)
    {
        // Read the LDR and divide it by 100
        uint16_t ldr_value = ldr_read() / 100;
        // Read the trimpot and divide it by 100
        uint16_t threshold = trimpot_read() / 100;
        // Display threshold on the seven segment display
        VPORTC.OUT = digit[threshold];
        
        // Checks if LDR value is less than or equal to the threshold value
        if(!g_neutral && ldr_value <= threshold)
        {
            g_press = 1;
            // Wait for the clock to stabilise
            while(RTC.STATUS > 0);
            // Set perioid to 8192 for 250ms delay before clicking spacebar
            RTC.PER = 8192;
        }
    }
}


ISR(RTC_CNT_vect)
{
    // Clear interrupt flags
    RTC.INTFLAGS = RTC_OVF_bm;
    // Check if the servo should press the spacebar
    if(!g_neutral && g_press)
    {
        // Set servo to 45 degree
        TCA0.SINGLE.CMP2BUF = SERVO_PWM_DUTY_MAX;
        // Set g_press to 0 after servo is operated
        g_press = 0;
        // Set g_neutral to 1 to set the servo to neutral position
        g_neutral = 1;
        // Wait for the clock to stabilise
        while(RTC.STATUS > 0);
        // Set perioid to 4096 to wait for ~100ms
        RTC.PER = 4096;
    }
    else
    {
        // Set servo to neutral position
        TCA0.SINGLE.CMP2BUF = SERVO_PWM_DUTY_NEUTRAL;
        // Set g_neutral to 0 after servo is returned to neutral position
        g_neutral = 0;
    }
    
}
