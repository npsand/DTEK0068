/* 
 * File:   ADC.h
 * Author: Nevil Sandaradura 
 * Email: npsand@utu.fi
 * Device:  ATmega4809 Curiosity Nano
 *
 * Created on December 9, 2021, 4:20 PM
 */

#ifndef ADC_H
#define	ADC_H

// To use SemaphoreHandle_t
#include "semphr.h" 

// Declare mutex for accessing adc reading
SemaphoreHandle_t mutex;
// Declare ADC initialize function
void adc_init(void);
// Struc for ADC readings
typedef struct {
    uint16_t ldr;
    uint16_t ntc;
    uint16_t pot;
}ADC_result_t;
// Declare ADC read fucntion, returns struct
ADC_result_t adc_read(void);

#endif	/* ADC_H */