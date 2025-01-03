#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif
#include <cr_section_macros.h>
//GPIO registers
#define FIO0PIN0 	(*(volatile unsigned char *)0x2009C014) //For Port 0, GPIO pins 0.0-0.7
#define FIO0PIN 	(*(volatile unsigned int *)0x2009C014) //For Port 0, GPIO pins 0.8-0.10
#define FIO0DIR		(*(volatile unsigned int *)0x2009C000)
#define FIO2DIR 	(*(volatile unsigned int *)0x2009c040)
#define FIO2PIN 	(*(volatile unsigned int *)0x2009c054)
#define FIO2CLR 	(*(volatile unsigned int *)0x2009C05C)
#define FIO2SET 	(*(volatile unsigned int *)0x2009C058)

//Timer 1 Registers
#define T0PR  		(*(volatile unsigned int *)0x4000400C)
#define T0MR0  		(*(volatile unsigned int *)0x40004018) //Timer0 Match register 0
#define T0MCR  		(*(volatile unsigned int *)0x40004014)
#define T0TCR 		(*(volatile unsigned int *)0x40004004)
#define T0IR   		(*(volatile unsigned int *)0x40004000) //Timer0 Interrupt register

//Timer 2 Registers
#define T2TCR 		(*(volatile unsigned int *)0x40090004)
#define T2TC 		(*(volatile unsigned int *)0x40090008) //register for Timer2 wait function

//General Purpose registers
#define PCONP    	(*(volatile unsigned int *)0x400FC0C4) //Power configuration register
#define ISER0   	(*(volatile unsigned int *)0xE000E100) //Interrupt set register
#define PCLKSEL0    (*(volatile unsigned int *)0x400FC1A8) //Peripheral Clock Select register
#define PINSEL1 	(*(volatile unsigned int *)0x4002C004)

//Digital to Analog registers
#define DACCTRL 	(*(volatile unsigned int *)0x4008C004) //DAC control register
#define DACCNTVAL 	(*(volatile unsigned int *)0x4008C008) //DAC control value register
#define DACR 		(*(volatile unsigned int *)0x4008C000)

bool enable;

int buttonStates[8];

int points = 0;

void wait_us(unsigned int us)
{
	unsigned int start;
	start = T2TC;
	T2TCR |= (1<<0); // make sure timer is ticking

	while((T2TC - start) < us){
		// do nothing
	}
}

//Controller Functions
void initPort(void){
	FIO2DIR |= (1<<5); // Latch output
	FIO2DIR |= (1<<6); // Clock output
	FIO2DIR &= ~(1<<7); // Data Input

	FIO2CLR = (1<<5) | (1<<6); // Ensures that latch and clock start low
}
void activateLatch() {
	FIO2PIN |= (1 << 5);
	wait_us(12);
	FIO2PIN &= (1 << 5);

}
void readButtonStates(int buttonStates[8])
{

	FIO2PIN |= (1<<5); // Latch goes high
	wait_us(12);
	FIO2PIN &= ~(1<<5); // Latch goes low


	for(int i = 0; i < 8; i++)
	{

		if(((FIO2PIN >> 7) & 1) == 0) // Reads the datapin
		{
			buttonStates[i] = 0;
		}
		else if(((FIO2PIN >> 7) & 1) == 1)
		{
			buttonStates[i] = 1;
		}
		else
		{
			buttonStates[i] = -1;
		}

		FIO2PIN |= (1<<6); // Clock goes high
		wait_us(6);
		FIO2PIN &= ~(1<<6); // Clock goes low
		wait_us(6); 		// This imitates the clock that the console would give out to shift the register through the 8 bits

	}
}
bool checkButton(char letter){
	switch (letter){
		case 'A' : {
			if(buttonStates[0] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'B' :{
			if(buttonStates[1] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'U' :{
			if(buttonStates[4] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'D' :{
			if(buttonStates[5] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'L' :{
			if(buttonStates[6] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'R' :{
			if(buttonStates[7] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'S' :{
			if(buttonStates[3] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
		}
		case 'X' :{
			if(buttonStates[2] == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
				}
		default : {
			return 0;
		}

	}
}

//LCD functions
void LCDwriteCommand(int commandVal){ //Make sure value is under 255 or else the value will be 0
	/*Pins 0.0 - 0.7 are for DB0-7 respectively
	Pin 0.8 is for R/S
	Pin 0.9 is for E (enable signal) */
	FIO0PIN0 = commandVal; //Assign data to DB0-7
	FIO0PIN &= ~(1 << 8); //Drive R/S low
	FIO0PIN |= (1 << 9);  //Drive E high then low to pulse
	FIO0PIN &= ~(1 << 9); //Finish pulse
	wait_us(100); //wait for command to complete
}
void LCDwriteData(int value){ //Make sure value is under 255 or else the value will be 0
	/*Pins 0.0 - 0.7 are for DB0-7 respectively
	Pin 0.8 is for R/S
	Pin 0.9 is for E (enable signal) */
	FIO0PIN0 = value; //assign ASCII value to DB0-7
	FIO0PIN |= (1 << 8);  //Drive R/S high to indicate this is data
	FIO0PIN |= (1 << 9);  //Drive E high then low to pulse
	FIO0PIN &= ~(1 << 9); //Finish pulse
	wait_us(100); //wait for data to be processed
}
void LCDinitialization(void){
	FIO0DIR = 1023; //000...00011111111111, a 1 for the first 11 pins on the port to enable all as output.
	FIO0PIN &= ~(1 << 8); //Drive R/S low to prep for a pulse
	FIO0PIN &= ~(1 << 9); //Drive E low to prep for a pulse
	wait_us(4000);
	LCDwriteCommand(0x38); //write command 0x38
	LCDwriteCommand(0x06); //write command 0x06
	LCDwriteCommand(0x0c); //write command 0x0f, Display is on, and cursor is visible and blinking, can change, see notes on LCD
	LCDwriteCommand(0x01); //write command 0x01, clears display and moves cursor to top left character.
	wait_us(4000);
}
void LCDarrowInitialization(void){
	//Right Arrow, At 0x00
	LCDwriteCommand(0x40);
	LCDwriteData(0b00000);
	LCDwriteData(0b00100);
	LCDwriteData(0b00010);
	LCDwriteData(0b11111);
	LCDwriteData(0b11111);
	LCDwriteData(0b00010);
	LCDwriteData(0b00100);
	LCDwriteData(0b00000);
	//Left Arrow, At 0x01 0x48
	LCDwriteData(0b00000);
	LCDwriteData(0b00100);
	LCDwriteData(0b01000);
	LCDwriteData(0b11111);
	LCDwriteData(0b11111);
	LCDwriteData(0b01000);
	LCDwriteData(0b00100);
	LCDwriteData(0b00000);
	//Down Arrow, At 0x02, 0x50
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b10101);
	LCDwriteData(0b01110);
	LCDwriteData(0b00100);
	LCDwriteData(0b00000);
	//Up Arrow, at 0x03, 0x58
	LCDwriteData(0b00100);
	LCDwriteData(0b01110);
	LCDwriteData(0b10101);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	//Right Arrow with underline, At 0x04
	LCDwriteData(0b00000);
	LCDwriteData(0b00100);
	LCDwriteData(0b00010);
	LCDwriteData(0b11111);
	LCDwriteData(0b11111);
	LCDwriteData(0b00010);
	LCDwriteData(0b00100);
	LCDwriteData(0b11111);
	//Left Arrow with underline, At 0x05 0x48
	LCDwriteData(0b00000);
	LCDwriteData(0b00100);
	LCDwriteData(0b01000);
	LCDwriteData(0b11111);
	LCDwriteData(0b11111);
	LCDwriteData(0b01000);
	LCDwriteData(0b00100);
	LCDwriteData(0b11111);
	//Down Arrow with underline, At 0x06,
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b10101);
	LCDwriteData(0b01110);
	LCDwriteData(0b00100);
	LCDwriteData(0b11111);
	//Up Arrow with underline, at 0x07,
	LCDwriteData(0b00100);
	LCDwriteData(0b01110);
	LCDwriteData(0b10101);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b00100);
	LCDwriteData(0b11111);
	LCDwriteCommand(0x80);
}
void LCDclear(void){
	LCDwriteCommand(0x01);
	wait_us(4001);
}
int LCDarrowValue(char letter){
	switch (letter) {
	case 'R' : return 0x00;
	case 'L' : return 0x01;
	case 'D' : return 0x02;
	case 'U' : return 0x03;
	case '0' : return 0x20;
	default : return 0x20;
	}
}

void intToString(int number, char* buffer) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (number == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (number < 0) {
        isNegative = 1;
        number = -number;
    }

    // Extract digits in reverse order
    while (number > 0) {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    }

    // Add '-' for negative numbers
    if (isNegative) {
        buffer[i++] = '-';
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string to get the correct order
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = buffer[j];
        buffer[j] = buffer[k];
        buffer[k] = temp;
    }
}
void LCDstartScreen(void)
{
	// Clear screen
	LCDclear();

	// First Line
	LCDwriteCommand(0x80);
	LCDwriteData('D');
	LCDwriteData('A');
	LCDwriteData('N');
	LCDwriteData('C');
	LCDwriteData('E');

	// Second Line
	LCDwriteCommand(0xC0); // Move cursor to the second row
	LCDwriteData('D');
	LCDwriteData('A');
	LCDwriteData('N');
	LCDwriteData('C');
	LCDwriteData('E');

	// Third Line
	LCDwriteCommand(0x94);
	LCDwriteData('R');
	LCDwriteData('E');
	LCDwriteData('V');
	LCDwriteData('O');
	LCDwriteData('L');
	LCDwriteData('U');
	LCDwriteData('T');
	LCDwriteData('I');
	LCDwriteData('O');
	LCDwriteData('N');
	LCDwriteData('!');

	// Fourth Line
	LCDwriteCommand(0xD4);
	LCDwriteData('P');
	LCDwriteData('r');
	LCDwriteData('e');
	LCDwriteData('s');
	LCDwriteData('s');
	LCDwriteData(' ');
	LCDwriteData('A');
	LCDwriteData(' ');
	LCDwriteData('b');
	LCDwriteData('u');
	LCDwriteData('t');
	LCDwriteData('t');
	LCDwriteData('o');
	LCDwriteData('n');
	LCDwriteData('!');

	while(1)
	{
		readButtonStates(buttonStates);
		if(checkButton('A'))
		{
			return; // Exit the function after A is pressed
		}
	}
}

void LCDendScreen(int points)
{
	char pointsBuffer[16]; // Storage for points

	// Convert the integer points to a string
	intToString(points, pointsBuffer);


	// Clear screen
	LCDclear();

	// First Line
	LCDwriteCommand(0x80);
	LCDwriteData('Y');
	LCDwriteData('O');
	LCDwriteData('U');
	LCDwriteData('R');
	LCDwriteData(' ');
	LCDwriteData('S');
	LCDwriteData('C');
	LCDwriteData('O');
	LCDwriteData('R');
	LCDwriteData('E');
	LCDwriteData(':');

	// Second Line
	LCDwriteCommand(0xC0);
	for(int i = 0; pointsBuffer[i] != '\0'; i++)
	{
		LCDwriteData(pointsBuffer[i]);
	}

	// Third Line
	LCDwriteCommand(0x94);
	LCDwriteData('P');
	LCDwriteData('r');
	LCDwriteData('e');
	LCDwriteData('s');
	LCDwriteData('s');
	LCDwriteData(' ');
	LCDwriteData('A');
	LCDwriteData(' ');
	LCDwriteData('T');
	LCDwriteData('o');
	LCDwriteData(' ');
	LCDwriteData('R');
	LCDwriteData('e');
	LCDwriteData('s');
	LCDwriteData('t');
	LCDwriteData('a');
	LCDwriteData('r');
	LCDwriteData('t');

	while(1){
		readButtonStates(buttonStates);
		if(checkButton('A'))
			{
				return;
			}
			}
}

//Timer0 functions
void Timer0Init(void){
	PCONP |= (1<<1);
	PCLKSEL0 &= ~(1<<3) & ~(1<<2);
	T0PR = 1; //1 clock cycles @1Mhz = 1 uS
	T0MR0 = 361; //value to change frequency of clock.
	T0MCR |= (1<<0) | (1<<1);
	T0TCR |= (1<<1);
	ISER0 |= (1<<1); //enable interrupts for Timer2
	T0TCR = 0x01;
	FIO0DIR |= (1 << 25); //Set P0.25 as an output for Timer Square wave
}
extern "C" void TIMER0_IRQHandler(void)
{
// Only need to check timer’s IR if using multiple
// interrupt conditions with the same timer
		T0IR = (1<<0); // clear MR0 event
		if (enable == 1){
			if (((FIO0PIN >> 25) & 1) == 1) { //functions as a square wave generator
						FIO0PIN &= ~(1<<25);
					}
					else {
						FIO0PIN |= (1 << 25);
					}
		}
}
void Timer0freq(int value){
	T0TCR |= (1<<1);
	T0MR0 = value;
	T0TCR &= ~(1<<1);
	/* Note Values
	* Bb = 270 (933Hz)
	* Ab = 304 (829Hz)
	* G = 322 (783Hz)
	* F = 361 (698.8Hz)
	* Eb = 406 (622Hz)
	* D = 430 (586Hz)
	* Db = 455 (544.4Hz)
	* C = 482 (523.3Hz)
	* Bb = 542 (465.7Hz)
	*/
}
//DAC
void DACinitialization(void){
	PINSEL1 |= (1 << 21); //Set AOUT
	PINSEL1 &= ~(1 << 20); //Set AOUT
	DACCNTVAL = 10; //set double-buffer interval for DAC
	DACCTRL |= (1<<1) | (1<<2); //enable double-buffering for DAC
}
void DACoutput(int value){
	int temp = value;
	while (temp > 0){
		DACR = value << 6; //assign value to DAC to output on AOUT
		temp = temp - 1; //decrement value to create sawtooth
		wait_us(1000); //wait. This extends or detracts the length of the sound.
	}

	//If we remove the decrement, put something here to set DACR to 0 after we finish waiting
}

void playNote(char value){ //, int length){
	/* Tempo ~= 120
	 * Eighth Note = 250; (8)
	 * Quarter Note = 500; (1)
	 * Half Note = 1000; (2)
	 * Dotted Half = 1500; (3)
	 * Whole = 2000; (4)
	 */

	/* Note Values
	* Bb = 270 (933Hz) ('B')
	* Ab = 304 (829Hz) ('A')
	* G = 322 (783Hz) ('G')
	* F = 361 (698.8Hz) ('F')
	* Eb = 406 (622Hz) ('E')
	* D = 430 (586Hz)
	* Db = 455 (544.4Hz) ('D')
	* C = 482 (523.3Hz) ('C')
	* Bb = 542 (465.7Hz) ('b')
	*/
	int noteTone;
	//switch(length){
	//	case 8: noteLength = 250; break;
	//	case 7: noteLength = 375; break;
	//	case 1: noteLength = 500; break;
	//	case 9: noteLength = 750; break;
	//	case 2: noteLength = 1000; break;
	//	case 3: noteLength = 1400; break;
	//}
	switch(value){
		case 'B': noteTone = 270; break;
		case 'A': noteTone = 304; break;
		case 'G': noteTone = 322; break;
		case 'F': noteTone = 361; break;
		case 'E': noteTone = 406; break;
		case 'D': noteTone = 455; break;
		case 'C': noteTone = 482; break;
		case 'b': noteTone = 542; break;
		default : noteTone = 0; break;
		}
	Timer0freq(noteTone);
	//enable = 1;
	//DACoutput(noteLength);
	//enable = 0;

}

//Arrays used to store the sequence of note length, note pitch, and how many notes are in each phrase.
int lengths [152]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* phrase 1 */8, 1, 8, 1, 1, 1, 2, 0,/* phrase 2 */ 8, 8, 8, 8, 1, 8, 8, 3, 0,/* phrase 3 */8, 1, 8, 1, 1, 1, 2, 0,/* phrase 4 */8, 8, 8, 8, 1, 8, 8, 3, 0,/* phrase 5 */ 8, 1, 8, 1, 1, 1, 2, 0, /* phrase 6 */8, 8, 8, 8, 1, 8, 8, 3, 0,/* phrase 7 */ 8, 1, 8, 1, 1, 1, 2, 0,/* phrase 8 */8, 8, 8, 8, 1, 1, 3, 0,/* phrase 9 */ 1, 1, 1, 1, 8, 1, 9, 0,/* phrase 10 */1, 1, 1, 1, 3, 0,/* phrase 11 */1, 1, 1, 1, 1, 8, 9, 0,/* phrase 12 */ 1, 1, 1, 1, 1, 8, 9, 0,/* phrase 13 */ 8, 1, 8, 1, 1, 1, 2, 0,/* phrase 14 */8, 8, 8, 8, 1, 8, 8, 3, 0,/* phrase 15 */8, 1, 8, 1, 1, 1, 2, 0,/* phrase 16 */8, 8, 8, 8, 1, 1, 3, 0, 0};
char notes [152]= {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 'E', 'F', 'E', 'C', 'A', 'F', 'E', '0', 'E', 'F', 'E', 'F', 'E', 'A', 'A', 'G', '0', 'D', 'E', 'D', 'b', 'G', 'F', 'E', '0', 'E', 'F', 'E', 'F', 'E', 'F', 'F', 'C', '0', 'E', 'F', 'E', 'C', 'A', 'F', 'E', '0', 'E', 'F', 'E', 'F', 'E', 'A', 'A', 'G', '0', 'D', 'E', 'D', 'b', 'G', 'F', 'E', '0', 'E', 'F', 'E', 'F', 'E', 'B', 'A', '0', 'F', 'F', 'A', 'F', 'E', 'C', 'E', '0', 'D', 'F', 'E', 'D', 'C', '0', 'C', 'D', 'E', 'F', 'G', 'G', 'G', '0', 'A', 'G', 'F', 'E', 'D', 'C', 'b', '0', 'E', 'F', 'E', 'C', 'A', 'F', 'E', '0', 'E', 'F', 'E', 'F', 'E', 'A', 'A', 'G', '0', 'D', 'E', 'D', 'b', 'G', 'F', 'E', '0', 'E', 'F', 'E', 'F', 'E', 'B', 'A', '0', '0'};
int groups [16]= {7, 8, 7, 8, 7, 8, 7, 7, 7, 5, 7, 7, 7, 8, 7, 7};

char row1[152] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', /* phrase 1 */'0', '0', '0', '0', 'R', '0', '0', '0', /* phrase 2 */'0', '0', '0', '0', '0', 'R', 'R', '0', '0', /* phrase 3 */'0', '0', '0', '0', 'R', '0', '0', '0', /* phrase 4 */'0', '0', '0', '0', '0', 'R', 'R', '0', '0', /* phrase 5 */'0', '0', '0', '0', 'R', '0', '0', '0', /* phrase 6 */'0', '0', '0', '0', '0', 'R', 'R', '0', '0', /* phrase 7 */'0', '0', '0', '0', 'R', '0', '0', '0', /* phrase 8 */'0', '0', '0', '0', '0', 'R', '0', '0', /* phrase 9 */'0', '0', 'R', '0', '0', '0', '0', '0', /* phrase 10 */'0', 'R', '0', '0', '0', '0', /* phrase 11 */'0', '0', '0', 'R', '0', '0', '0', '0', /* phrase 12 */'R', 'R', 'R', '0', '0', '0', '0', '0', /* phrase 13 */'0', '0', '0', '0', 'R', '0', '0', '0', /* phrase 14 */'0', '0', '0', '0', '0', 'R', 'R', '0', '0', /* phrase 15 */'0', '0', '0', '0', 'R', '0', '0', '0', /* phrase 16 */'0', '0', '0', '0', '0', 'R', '0', '0', '0'};
char row2[152] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', /* phrase 1 */'0', 'U', '0', '0', '0', 'U', '0', '0', /* phrase 2 */'0', '0', '0', '0', '0', '0', '0', 'U', '0', /* phrase 3 */'0', 'U', '0', '0', '0', 'U', '0', '0', /* phrase 4 */'0', '0', '0', '0', '0', '0', '0', 'U', '0', /* phrase 5 */'0', 'U', '0', '0', '0', 'U', '0', '0', /* phrase 6 */'0', '0', '0', '0', '0', '0', '0', 'U', '0', /* phrase 7 */'0', 'U', '0', '0', '0', 'U', '0', '0', /* phrase 8 */'0', '0', '0', '0', '0', '0', 'U', '0', /* phrase 9 */'U', 'U', '0', 'U', '0', '0', '0', '0', /* phrase 10 */'0', '0', 'U', '0', '0', '0', /* phrase 11 */'0', '0', 'U', '0', '0', '0', '0', '0', /* phrase 12 */'0', '0', '0', 'U', '0', '0', '0', '0', /* phrase 13 */'0', 'U', '0', '0', '0', 'U', '0', '0', /* phrase 14 */'0', '0', '0', '0', '0', '0', '0', 'U', '0', /* phrase 15 */'0', 'U', '0', '0', '0', 'U', '0', '0', /* phrase 16 */'0', '0', '0', '0', '0', '0', 'U', '0', '0'};
char row3[152] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', /* phrase 1 */'D', '0', 'D', '0', '0', '0', 'D', '0', /* phrase 2 */'0', 'D', '0', 'D', '0', '0', '0', '0', '0', /* phrase 3 */'D', '0', 'D', '0', '0', '0', 'D', '0', /* phrase 4 */'0', 'D', '0', 'D', '0', '0', '0', '0', '0', /* phrase 5 */'D', '0', 'D', '0', '0', '0', 'D', '0', /* phrase 6 */'0', 'D', '0', 'D', '0', '0', '0', '0', '0', /* phrase 7 */'D', '0', 'D', '0', '0', '0', 'D', '0', /* phrase 8 */'0', 'D', '0', 'D', '0', '0', '0', '0', /* phrase 9 */'0', '0', '0', '0', 'D', '0', '0', '0', /* phrase 10 */'0', '0', '0', 'D', '0', '0', /* phrase 11 */'0', 'D', '0', '0', '0', '0', '0', '0', /* phrase 12 */'0', '0', '0', '0', 'D', '0', '0', '0', /* phrase 13 */'D', '0', 'D', '0', '0', '0', 'R', '0', /* phrase 14 */'0', 'D', '0', 'D', '0', '0', '0', '0', '0', /* phrase 15 */'D', '0', 'D', '0', '0', '0', 'D', '0', /* phrase 16 */'0', 'D', '0', 'D', '0', '0', '0', '0', '0'};
char row4[152] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', /* phrase 1 */'0', '0', '0', 'L', '0', '0', '0', '0', /* phrase 2 */'L', '0', 'L', '0', 'L', '0', '0', '0', '0', /* phrase 3 */'0', '0', '0', 'L', '0', '0', '0', '0', /* phrase 4 */'L', '0', 'L', '0', 'L', '0', '0', '0', '0', /* phrase 5 */'0', '0', '0', 'L', '0', '0', '0', '0', /* phrase 6 */'L', '0', 'L', '0', 'L', '0', '0', '0', '0', /* phrase 7 */'0', '0', '0', 'L', '0', '0', '0', '0', /* phrase 8 */'L', '0', 'L', '0', 'L', '0', '0', '0', /* phrase 9 */'0', '0', '0', '0', '0', 'L', 'L', '0', /* phrase 10 */'L', '0', '0', '0', 'L', '0', /* phrase 11 */'L', '0', '0', '0', 'L', 'L', 'L', '0', /* phrase 12 */'0', '0', '0', '0', '0', 'L', 'L', '0', /* phrase 13 */'0', '0', '0', 'L', '0', '0', '0', '0', /* phrase 14 */'L', '0', 'L', '0', 'L', '0', '0', '0', '0', /* phrase 15 */'0', '0', '0', 'L', '0', '0', '0', '0', /* phrase 16 */'L', '0', 'L', '0', 'L', '0', '0', '0', '0'};

int main(void) {

	PCONP |= (1<<22); //enable power to Timer 2
  	initPort();
  	LCDinitialization();
  	LCDarrowInitialization();
  	LCDclear();
  	Timer0Init();
	DACinitialization();
	while(1) {
		LCDstartScreen();
		points = 0;
		for (int i=0; i<151; i++)
		{
			//Update Screen
			LCDclear();
			for (int k = i; k<20+i; k++){
				if (k == i){
					if (row1[k] == '0'){
						LCDwriteData('_');
					}
					else {LCDwriteData(LCDarrowValue(row1[k]) + 4);}
				}
				else {
					LCDwriteData(LCDarrowValue(row1[k]));
				}
			}
			for (int k = i; k<20+i; k++){
				if (k == i){
					if (row3[k] == '0'){
						LCDwriteData('_');
					}
					else {LCDwriteData(LCDarrowValue(row3[k]) + 4);}
				}
				else {
					LCDwriteData(LCDarrowValue(row3[k]));
				}
			}
			for (int k = i; k<20+i; k++){
				if (k == i){
					if (row2[k] == '0'){
						LCDwriteData('_');
					}
					else {LCDwriteData(LCDarrowValue(row2[k]) + 4);}
				}
				else {
					LCDwriteData(LCDarrowValue(row2[k]));
				}
			}
			for (int k = i; k<20+i; k++){
				if (k == i){
					if (row4[k] == '0'){
						LCDwriteData('_');
					}
					else {LCDwriteData(LCDarrowValue(row4[k]) + 4);}
				}
				else {
					LCDwriteData(LCDarrowValue(row4[k]));
				}
			}
			//Set frequency and length for current note
			int noteLength;
			switch(lengths[i]){
				case 8: noteLength = 250; break;
				case 7: noteLength = 375; break;
				case 1: noteLength = 500; break;
				case 9: noteLength = 750; break;
				case 2: noteLength = 1000; break;
				case 3: noteLength = 1400; break;
				default : noteLength = 500; break;
			}
			playNote(notes[i]);
			//Receive input and play note
			if (((row1[i] == '0') && (row2[i] == 0)) && ((row3[i] == '0') && (row4[i] == 0))){
				enable = 0;
			}
			else {
				enable = 1;
			}
			bool hasHit = 0;
			DACR = 50 << 6;
			for (int k = 0; k<10000; k++){
				if ((k == noteLength) | (k > noteLength)){
					break;
				}
				else {
					readButtonStates(buttonStates);
					if ((hasHit == 1) || ((hasHit = 0) && (k >= noteLength/3))){}
					else if ((row1[i] != '0') & (checkButton('R') == 1)){
						DACR = 300 << 6;
						points += 1;
						hasHit = 1;
					}
					else if ((row2[i] != '0') & (checkButton('U') == 1)){
						DACR = 300 << 6;
						points += 1;
						hasHit = 1;
					}
					else if ((row3[i] != '0') & (checkButton('D') == 1)){
						DACR = 300 << 6;
						points += 1;
						hasHit = 1;
					}
					else if ((row4[i] != '0') & (checkButton('L') == 1)){
						DACR = 300 << 6;
						points += 1;
						hasHit = 1;
					}
				}
				wait_us(10);
			}
			enable = 0;
		}
		LCDendScreen(points);
	}
	return 0 ;
}
