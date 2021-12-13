/*
 * File:   ADC.c
 * Author: Nevil Sandaradura
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 * 
 * Read values from POT, NCT and LDR
 * 
 * Created on December 9, 2021, 4:20 PM
 */


#include <avr/io.h>
// FreeRTOS
#include "FreeRTOS.h"
// Include adc.h for use E.g. ADC_result_t struct
#include "adc.h"
// Convert function that does ADC converts, takes muxpos as parameter
uint16_t adc_convert(register8_t muxpos)
{
        // Clear REFSEL
        ADC0.CTRLC &= ~ADC_REFSEL_VDDREF_gc;
        // Set muxpos to parameter value
        ADC0.MUXPOS = muxpos;
        // Use internal reference voltage (2.5v)
        ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;
        // Start conversion
        ADC0.COMMAND = ADC_STCONV_bm;
        // Wait for ADC0.INTFLAGS to get set
        while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
        // Return read value
        return ADC0.RES;
}
// Function which reads LDR, NCT and POT, and returns struct
ADC_result_t adc_read(void)
{
    ADC_result_t adc_result;
    
    adc_result.ldr = adc_convert(ADC_MUXPOS_AIN8_gc);
    adc_result.ntc = adc_convert(ADC_MUXPOS_AIN9_gc);
    adc_result.pot = adc_convert(ADC_MUXPOS_AIN14_gc);
    
    return adc_result;
}

void adc_init(void)
{
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
    
    // POTENTIOMETER
    // Set PF4 (AIN14) as input 
    PORTF.DIRCLR = PIN4_bm;
    // Disable input buffer
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
    
    // THERMISTOR
    // Set PE1 (AIN9) as input 
    PORTE.DIRCLR = PIN1_bm;
    // Disable input buffer
    PORTE.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
}

