/*
 * File:   main.c
 * Author: Nevil Sandaradura
 *
 * Created on October 27, 2021, 2:43 PM
 */


#include <avr/io.h>

int main(void) 
{
    PORTF.DIR = PORTF.DIR | PIN5_bm;
    PORTF.DIR = PORTF.DIR & ~PIN6_bm;
    
    while (1) 
    {
        if(PORTF.IN & PIN6_bm)
        {
            PORTF.OUT = PORTF.OUT | PIN5_bm;
        }else
        {
            PORTF.OUT = PORTF.OUT & ~PIN5_bm;
        }
    }
}
