/*
    Author: Brandon Webb
    Completed: Fall 2024
    Project Overview: This project involves creating a game of Tic-Tac-Toe using a 3-by-3 grid where two players alternate turns. To implement 
        this functionality, the I/O subsystem of the LPC1769 microcontroller is used to interface with nine switches and nine 
        dual-color LEDs that toggle between yellow and green, representing each player's moves. 

        The game manages three possible outcomes: a yellow win, a green win, or a tie. A player wins by achieving three 
        consecutive markers of the same color in a horizontal, vertical, or diagonal line. Upon reaching a win or tie 
        condition, the board pauses for a specified duration before resetting to its initial state to start a new game. 

        Additional safeguards ensure fair play: players can only select unoccupied positions and cannot overwrite an 
        already-claimed spot.
 */




#define FIO0DIR (*(volatile unsigned int *)0x2009c000)
#define FIO0PIN (*(volatile unsigned int *)0x2009c014)
#define FIO2DIR (*(volatile unsigned int *)0x2009c040)
#define FIO2PIN (*(volatile unsigned int *)0x2009c054)
#define PINMODE0 (*(volatile unsigned int *)0x4002C040)
#define PINMODE1 (*(volatile unsigned int *)0x4002C044)


#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif
#include <cr_section_macros.h>
#include <stdbool.h>

void initPort(void){
    FIO0DIR &= ~(1<<1);
    FIO0DIR &= ~(1<<18);
    FIO0DIR &= ~(1<<17);
    FIO0DIR &= ~(1<<15);
    FIO0DIR &= ~(1<<16);
    FIO0DIR &= ~(1<<23);
    FIO0DIR &= ~(1<<24);
    FIO0DIR &= ~(1<<25);
    FIO0DIR &= ~(1<<26);

	FIO0DIR |= (1<<4);
	FIO0DIR |= (1<<10);
	FIO2DIR |= (1<<0);
	FIO2DIR |= (1<<2);
	FIO2DIR |= (1<<4);
	FIO2DIR |= (1<<6);
	FIO2DIR |= (1<<8);
	FIO2DIR |= (1<<11);
	FIO0DIR |= (1<<2);

	FIO0DIR |= (1<<5);
	FIO0DIR |= (1<<11);
	FIO2DIR |= (1<<1);
	FIO2DIR |= (1<<3);
	FIO2DIR |= (1<<5);
	FIO2DIR |= (1<<7);
	FIO2DIR |= (1<<10);
	FIO2DIR |= (1<<12);
	FIO0DIR |= (1<<3);

	PINMODE0 |= (1<<3) | (1<<2);
	PINMODE1 |= (1<<5) | (1<<4);
	PINMODE1 |= (1<<3) | (1<<2);
	PINMODE0 |= (1<<31) | (1<<30);
	PINMODE1 |= (1<<1) | (1<<0);
	PINMODE1 |= (1<<15) | (1<<14);
	PINMODE1 |= (1<<17) | (1<<16);
    PINMODE1 |= (1<<19) | (1<<18);
	PINMODE1 |= (1<<21) | (1<<20);
}
volatile int busy;
void wait(int ticks)
{
	volatile int count;
	for(count=0; count < ticks; count++)
	{
		// do nothing
	}
}

void turnOffAll(){
	FIO0PIN &= ~(1<<4);
	busy++;
	FIO0PIN &= ~(1<<10);
	busy++;
	FIO2PIN &= ~(1<<0);
	busy++;
	FIO2PIN &= ~(1<<2);
	busy++;
    FIO2PIN &= ~(1<<4);
	busy++;
	FIO2PIN &= ~(1<<6);
	busy++;
	FIO2PIN &= ~(1<<8);
	busy++;
	FIO2PIN &= ~(1<<11);
	busy++;
	FIO0PIN &= ~(1<<2);
	busy++;

	FIO0PIN &= ~(1<<5);
	busy++;
	FIO0PIN &= ~(1<<11);
	busy++;
	FIO2PIN &= ~(1<<1);
	busy++;
	FIO2PIN &= ~(1<<3);
	busy++;
	FIO2PIN &= ~(1<<5);
	busy++;
	FIO2PIN &= ~(1<<7);
	busy++;
	FIO2PIN &= ~(1<<10);
	busy++;
	FIO2PIN &= ~(1<<12);
	busy++;
	FIO0PIN &= ~(1<<3);
	busy++;
 }

void readSw(int *numptr){

	busy++;
	if (((FIO0PIN >> 1) & 1) == 1){ //switch 1 (P0.1)
		 numptr[0] = 0; numptr[1] = 0;
	  }
	 else if (((FIO0PIN >> 18) & 1) == 1){ //switch 2 (P0.18)
	 	 numptr[0] = 0; numptr[1] = 1;
	  }
	 else if (((FIO0PIN >> 17) & 1) == 1) { //switch 3 (P0.17)
	 	 numptr[0] = 0; numptr[1] = 2;
	  }
	 else if (((FIO0PIN >> 15) & 1) == 1) { //switch 4 (P0.15)
	 	 numptr[0] = 1; numptr[1] = 0;
	  	 }
	 else if (((FIO0PIN >> 16) & 1) == 1) { //switch 5 (P0.16)
	 	 numptr[0] = 1; numptr[1] = 1;
	  	 }
	 else if (((FIO0PIN >> 23) & 1) == 1) { //switch 6 (P0.23)
	 	 numptr[0] = 1; numptr[1] = 2;
	  	 }
	 else if (((FIO0PIN >> 24) & 1) == 1) { //switch 7 (P0.24)
	 	 numptr[0] = 2; numptr[1] = 0;
	  	 }
	 else if (((FIO0PIN >> 25) & 1) == 1 ) { //switch 8 (P0.25)
	 	 numptr[0] = 2; numptr[1] = 1;
	  	 }
	 else if (((FIO0PIN >> 26) & 1) == 1) { //switch 9 (P0.26)
	 	 numptr[0] = 2; numptr[1] = 2;
	  	 }
	 else {numptr[0] = -1; numptr[1] = -1;}
	  }

void turnOnY(int *numptr){

	if ((numptr[0] == 0) && (numptr[1] == 0)){
		FIO0PIN |= (1<<4);
	}
	else if ((numptr[0] == 0) && (numptr[1] == 1)){
		FIO0PIN |= (1<<10);
	}
	else if ((numptr[0] == 0) && (numptr[1] == 2)){
		FIO2PIN |= (1<<0);
	}
	else if ((numptr[0] == 1) && (numptr[1] == 0)){
		FIO2PIN |= (1<<2);
	}
	else if ((numptr[0] == 1) && (numptr[1] == 1)){
		FIO2PIN |= (1<<4);
	}
	else if ((numptr[0] == 1) && (numptr[1] == 2)){
		FIO2PIN |= (1<<6);
	}
	else if ((numptr[0] == 2) && (numptr[1] == 0)){
		FIO2PIN |= (1<<8);
	}
	else if ((numptr[0] == 2) && (numptr[1] == 1)){
		FIO2PIN |= (1<<11);
	}
	else if ((numptr[0] == 2) && (numptr[1] == 2)){
		FIO0PIN |= (1<<2);
	}
	busy++;
 }

void turnOnG(int *numptr){

	 if ((numptr[0] == 0) && (numptr[1] == 0)){
	 	FIO0PIN |= (1<<5);
	 }
	 else if ((numptr[0] == 0) && (numptr[1] == 1)){
	 	FIO0PIN |= (1<<11);
	 }
	 else if ((numptr[0] == 0) && (numptr[1] == 2)){
	 	FIO2PIN |= (1<<1);
	 }
	 else if ((numptr[0] == 1) && (numptr[1] == 0)){
	 	FIO2PIN |= (1<<3);
	 }
	 else if ((numptr[0] == 1) && (numptr[1] == 1)){
		FIO2PIN |= (1<<5);
	}
	 else if ((numptr[0] == 1) && (numptr[1] == 2)){
	 	FIO2PIN |= (1<<7);
	 }
	 else if ((numptr[0] == 2) && (numptr[1] == 0)){
		FIO2PIN |= (1<<10);
	}
	 else if ((numptr[0] == 2) && (numptr[1] == 1)){
	 	FIO2PIN |= (1<<12);
	 }
	 else if ((numptr[0] == 2) && (numptr[1] == 2)){
	 	FIO0PIN |= (1<<3);
	 }
	 busy++;
 }

bool checkBoard(int board[][3]){
	int one, two, three;
	//checking rows
	for (int i = 0; i < 3; i++){
	    	 one = board[i][0];
	    	 two = board[i][1];
	    	 three = board[i][2];

	    	 if ((one != 0) && (two != 0) && (three != 0) && ((one == two) == three)){
	    	 		return true;
	    	 	}
	     }
	//returns true if the row is the same
	//checking columns
	for (int i = 0; i < 3; i++){
		     one = board[0][i];
		     two = board[1][i];
		     three = board[2][i];

		     if ((one != 0) && (two != 0) && (three != 0) && ((one == two) == three)){
		    	 	return true;
		    	 	}

	}
	//returns true if the row is the same
	//checks diagonal from top left to bottom right
	if ((board[0][0] != 0) && (board[1][1] != 0) && (board[2][2] != 0) && ((board[0][0] == board[1][1]) == board[2][2])){
		return true;
	}
	//checks diagonal from bottom left to top right
	else if ((board[0][2] != 0) && (board[1][1] != 0) && (board[2][0] != 0) && ((board[0][2] == board[1][1]) == board[2][0])){
		return true;
	}
	return false;
}

bool checkFullBoard(int board [][3])
{
	bool full = true;

	//checking rows
	for (int i = 0; i < 3; i++){
		for (int j = 0; j<3; j++){
			if (board[i][j] == 0){
				full = false;
			}
		}
	}
 return full;
}

int main(void) {
     	initPort();
     	turnOffAll();
     	int num[2] = {-1 -1};
     	int* numptr = num;
     	int board[3][3]; //0 means the space is unoccupied, 1 means the space is occupied by yellow, 2 means the space               is occupied by green
    	int boardY[3][3]; //0 means the space is unoccupied, 1 means the space is occupied by yellow
int boardG[3][3]; //0 means the space is unoccupied, 1 means the space is occupied by green
int player = 1; //0 means someone wins or game is over, 1 means it's yellow player's turn, 2 means it's green player's turn

//clear board
     	for (int i = 0; i < 3; i++){
    		 for (int j = 0; j < 3; j++){
    			 boardY[i][j] = 0;
    		 	board[i][j] = 0;
    		 	boardG[i][j] = 0;
    		}
     	}

while (1){
    		 if (player == 1){
    			 readSw(numptr);
    		 	if (((numptr[0] != -1) && (numptr[1] != -1)) && ((boardY[numptr[0]][numptr[1]] == 0) && (boardG[numptr[0]][numptr[1]] == 0))){
    			 //turns on yellow LED in proper position
    			 turnOnY(numptr);
    			 //update board
    			 boardY[numptr[0]][numptr[1]] = 1; //make space occupied by yellow
    			 board[numptr[0]][numptr[1]] = 1;
    			 if ((checkBoard(boardY) == true) || (checkFullBoard(board) == true)){ //returns true if won
    				 player = 3;
    			 }
    			 else{
    				player = 2;
    			 }
    		 }
    		 wait(100); //roughly one second (idk)
    	 }

    	 	else if (player == 2){
    	     		 readSw(numptr);
    	     		 if (((numptr[0] != -1) && (numptr[1] != -1)) && ((boardY[numptr[0]][numptr[1]] == 0) && (boardG[numptr[0]][numptr[1]] == 0))){
    	     			 //turns on green LED in proper position
    	     			 turnOnG(numptr);
    	     			 //update board
    	     			 boardG[numptr[0]][numptr[1]] = 1; //make space occupied by green
    	     			 board[numptr[0]][numptr[1]] = 1;
    	     			 if ((checkBoard(boardG) == true) || (checkFullBoard(board) == true)){ //returns true if they won
    	     				 player = 3;
    	     			 }
    	     			 else{
    	     				 player = 1;
    	     			 }
    	     		 }
    	     		 wait(100); //roughly one second (idk)
    	     	 }

    	 else if (player == 3){
    		 wait(1000000);
    		 player = 1;
    		 //clear board
    		 for (int i = 0; i < 3; i++){
    		 	for (int j = 0; j < 3; j++){
    		     		boardY[i][j] = 0;
    		     		board[i][j] = 0;
    		     		boardG[i][j] = 0;
    		     	 }
    		      }
    		 turnOffAll();
    		 num[0] = -1; num[1] = -1;
    	 }
     }
} //main


