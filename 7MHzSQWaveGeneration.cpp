/* 
    Author: Brandon Webb
    Completed: Fall 2024
    Project Overview: This utilizes the LPC1769 microcontroller's Phase-Lock-Loop Subsytem to generate a 7 MHz square wave with a 50% duty cycle.
*/

#define PLL0CON (*(volatile unsigned int *)0x400FC080)
#define PLL0CFG (*(volatile unsigned int *)0x400FC084)
#define CLKSRCSEL (*(volatile unsigned int *)0x400FC10C)
#define CCLKCFG (*(volatile unsigned int *)0x400FC104)
#define PINSEL3 (*(volatile unsigned int *)0x4002C00C)
#define CLKOUTCFG (*(volatile unsigned int *)0x400FC1C8)
#define PLL0FEED (*(volatile unsigned int *)0x400FC08C)

#ifdef _USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdbool.h>

void wait(int ticks) {
    volatile int count;
    for (count = 0; count < ticks; count++) {
        // Do nothing
    }
}

int main(void) {
    PINSEL3 |= (1 << 22); // Enable output for clockout for Port 1.27
    CLKSRCSEL |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 5); // The M we decided to use

    PLL0CFG = (1 << 1) | (1 << 5);
    PLL0FEED = 0xAA; PLL0FEED = 0x55; // Feed Sequence for PLL0

    PLL0CON |= (1 << 0);
    PLL0FEED = 0xAA; PLL0FEED = 0x55; // Feed Sequence for PLL0

    wait(100000); // The wait function used to allow PLL0 to enable

    PLL0CON |= (1 << 1);
    PLL0FEED = 0xAA; PLL0FEED = 0x55; // Feed Sequence for PLL0

    CLKOUTCFG |= (1 << 8);

    return 0;
}
