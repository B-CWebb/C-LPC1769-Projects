/* 
    Author: Brandon Webb
    Completed: Fall 2024
    Project Overview: This project leverages the LPC1769 microcontroller's digital-to-analog subsystem to synthesize the sound of a doorbell 
        resonating at frequencies of 698 Hz and 587 Hz. A button press triggers the first "bell" tone, which begins at a maximum 
        amplitude of approximately 3.0 volts and gradually decrescendos to silence. Upon button release, the second "bell" tone 
        follows the same pattern. If the button is released before the first tone has completed, the second tone starts immediately, 
        ensuring seamless transitions between the two sounds.
*/


#define PINSEL1     (*(volatile unsigned int *)0x4002C004)
#define DACR        (*(volatile unsigned int *)0x4008C000)
#define T0TC        (*(volatile unsigned int *)0x40004008)
#define T0TCR       (*(volatile unsigned int *)0x40004004)
#define DACCTRL     (*(volatile unsigned int *)0x4008C004)
#define DACCNTVAL   (*(volatile unsigned int *)0x4008C008)
#define FIO2DIR     (*(volatile unsigned int *)0x2009C040)
#define FIO0PIN     (*(volatile unsigned int *)0x2009C014)
#define FIO0DIR     (*(volatile unsigned int *)0x2009C000)
#define T0TC        (*(volatile unsigned int *)0x40004008)
#define T0TCR       (*(volatile unsigned int *)0x40004004)
#define T0MR0       (*(volatile unsigned int *)0x40004018)
#define T0IR        (*(volatile unsigned int *)0x40004000)
#define T0MCR       (*(volatile unsigned int *)0x40004014)
#define T0PR        (*(volatile unsigned int *)0x4000400C)
#define ISER0       (*(volatile unsigned int *)0xE000E100)
#define PCONP       (*(volatile unsigned int *)0x400FC0C4)
#define PCLKSEL0    (*(volatile unsigned int *)0x400FC1A8)
#define PINMODE0    (*(volatile unsigned int *)0x4002C040)
#define PINMODE4    (*(volatile unsigned int *)0x4002C050)
#define T1PR        (*(volatile unsigned int *)0x4000800C)
#define T1MR0       (*(volatile unsigned int *)0x40008018)
#define T1MCR       (*(volatile unsigned int *)0x40008014)
#define T1TCR       (*(volatile unsigned int *)0x40008004)
#define T1IR        (*(volatile unsigned int *)0x40008000)
#define T2TCR       (*(volatile unsigned int *)0x40090004)
#define T2TC        (*(volatile unsigned int *)0x40090008)
#define ICER0       (*(volatile unsigned int *)0xE000E180)

#ifdef _USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdbool.h>

bool enableT0, enableT1 = 0; //Global variables used for Timer0 & 1 clocks that are changed in main()

void wait_us(int us) {
    int start;
    start = T2TC;
    T2TCR |= (1<<0); // make sure timer is ticking
    while((T2TC - start) < us) {
        // do nothing
    }
}

void Timer0Init(void) {
    PCONP |= (1<<1);
    PCLKSEL0 &= ~(1<<3) & ~(1<<2);
    T0PR = 1; //1 clock cycles @1Mhz = 1 uS
    T0MR0 = 361; //value to change frequency of clock.
    T0MCR |= (1<<0) | (1<<1);
    T0TCR |= (1<<1);
    ISER0 |= (1<<1); //enable interrupts for Timer2
    T0TCR = 0x01;
}

void Timer1Init(void) {
    PCONP |= (1<<2);
    PCLKSEL0 &= ~(1<<5) & ~(1<<4);
    T1PR = 1; //1 clock cycles @1Mhz = 1 uS
    T1MR0 = 430; //value to change frequency of clock.
    T1MCR |= (1<<0) | (1<<1);
    T1TCR |= (1<<1);
    ISER0 = (1<<2); //enable interrupts for Timer1
    T1TCR = 0x01;
}

extern "C" void TIMER0_IRQHandler(void) {
    // Only need to check timer’s IR if using multiple
    // interrupt conditions with the same timer
    T0IR = (1<<0); // clear MR0 event
    if (enableT0 == 1) {
        if (((FIO0PIN >> 9) & 1) == 1) { //functions as a square wave generator
            FIO0PIN &= ~(1<<9);
        } else {
            FIO0PIN |= (1 << 9);
        }
    }

extern "C" void TIMER1_IRQHandler(void) {
    // Only need to check timer’s IR if using multiple
    // interrupt conditions with the same timer
    T1IR = (1<<0); // clear MR0 event
    if (enableT1 == 1) {
        if (((FIO0PIN >> 9) & 1) == 1) { //functions as a square wave generator
            FIO0PIN &= ~(1<<9);
        } else {
            FIO0PIN |= (1 << 9);
        }
    }
}

int main(void) {
    Timer0Init();
    Timer1Init();
    PCONP |= (1<<22); //enable power to Timer 2
    ICER0 = (1<<3); //enable interrupts for Timer2

    FIO0DIR |= (1 << 9); //Set P0.9 as an output for Timer Square wave
    FIO0DIR &= ~(1<<5); //make P0.5 an input for button press
    PINMODE0 |= (1 << 11) | (1 << 10); //Pull down Resistor on P0.5
    PINSEL1 |= (1 << 21); //Set AOUT
    PINSEL1 &= ~(1 << 20); //Set AOUT

    DACCNTVAL = 10; //set double-buffer interval for DAC
    DACCTRL |= (1<<1) | (1<<2); //enable double-buffering for DAC

    int value; //variable assigned to D2A data register
    bool samp = 0; //variable to store button read on P0.5
    bool enablePressTone = 1; //variable to store that the button has been pressed
    bool enableReleaseTone = 0; //variable to enable the Release Tone

    while (1) {
        //read switch for press
        samp = (FIO0PIN >> 5) & 1;
        wait_us(1);

        //Press Tone generating if-statement
        if ((samp == 1) && (enablePressTone == 1)) {
            enableT1 = 0; //Ensures T1 clock signal is off
            enableT0 = 1; //turn on Timer square wave
            value = 1000; //Reset value to decrement for ~3.0V

            while (value > 0) {
                //reads in switch for release
                samp = (FIO0PIN >> 5) & 1;
                if (samp == 0) {
                    enableReleaseTone = 1;
                    enableT0 = 0; //turn off Timer square wave
                    break; //exits while loop if switch is released
                }
                DACR = value << 6; //assign value to DAC to output on AOUT
                value = value - 1; //decrement value
                wait_us(1000); //wait. This extends or detracts the length of the sound.
            }
            enablePressTone = 0;
            enableT0 = 0; //turn off Timer square wave
            while ((FIO0PIN >> 5) & 1) { //wait for button to be released
                wait_us(1);
            }
            enableReleaseTone = 1;
        }
    }
}
