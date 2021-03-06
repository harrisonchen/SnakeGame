/*	hchen030_lab8_LCD.c - $date$
 *	Name & E-mail:  - 
 *	CS Login: 
 *	Partner(s) Name & E-mail:  - 
 *	Lab Section: 
 *	Assignment: Lab #  Exercise # 
 *	Exercise Description:
 *	
 *	
 *	I acknowledge all content contained herein, excluding template or example 
 *	code, is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <ucr/bit.h>
#include <ucr/timer.h>
#include <stdio.h>

unsigned char LCD_counter;

unsigned long int findGCD(unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while (1){
		c = a%b;
	if (c==0) {return b;}
	a = b;
	b = c;
}
return 0;
}

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

unsigned char x = 0;

//********* LCD interface synchSM *********************************************

// Define LCD port assignments here so easier to change than if hardcoded below
unsigned char *LCD_Data = &PORTD;	// LCD 8-bit data bus
unsigned char *LCD_Ctrl = &PORTB;	// LCD needs 2-bits for control, use port B
const unsigned char LCD_RS = 3;		// LCD Reset pin is PB3
const unsigned char LCD_E = 4;		// LCD Enable pin is PB4

unsigned char LCD_rdy_g = 0; // Set by LCD interface synchSM, ready to display new string
unsigned char LCD_go_g = 0; // Set by user synchSM wishing to display string in LCD_string_g
unsigned char LCD_string_g[32]; // Filled by user synchSM, 16 chars plus end-of-string char

void LCD_WriteCmdStart(unsigned char cmd) {
	*LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_RS, 0);
	*LCD_Data = cmd;
	*LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 1);
}
void LCD_WriteCmdEnd() {
	*LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 0);
}
void LCD_WriteDataStart(unsigned char Data) {
	*LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_RS,1);
	*LCD_Data = Data;
	*LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 1);
}
void LCD_WriteDataEnd() {
	*LCD_Ctrl = SetBit(*LCD_Ctrl,LCD_E, 0);
}
void LCD_Cursor(unsigned char column ) {
	if ( column < 16 ) { // IEEE change this value to 16
		LCD_WriteCmdStart(0x80+column);
	}
	else {
		LCD_WriteCmdStart(0xBF+column); // IEEE change this value to 0xBF+column
	}
}

enum LI_States { LI_Init1, LI_Init2, LI_Init3, LI_Init4, LI_Init5, LI_Init6,
	LI_WaitDisplayString, LI_Clr, LI_PositionCursor, LI_DisplayChar, LI_WaitGo0 } LI_State;

void LI_Tick() {
	static unsigned char i;
	switch(LI_State) { // Transitions
		case -1:
			LI_State = LI_Init1;
			break;
		case LI_Init1:
			LI_State = LI_Init2;
			i=0;
			break;
		case LI_Init2:
			if (i<10) { // Wait 100 ms after power up
				LI_State = LI_Init2;
			}
			else {
				LI_State = LI_Init3;
			}
			break;
		case LI_Init3:
			LI_State = LI_Init4;
			LCD_WriteCmdEnd();
			break;
		case LI_Init4:
			LI_State = LI_Init5;
			LCD_WriteCmdEnd();
			break;
		case LI_Init5:
			LI_State = LI_Init6;
			LCD_WriteCmdEnd();
			break;
		case LI_Init6:
			LI_State = LI_WaitDisplayString;
			LCD_WriteCmdEnd();
			break;
		//////////////////////////////////////////////
		case LI_WaitDisplayString:
			if (!LCD_go_g) {
				LI_State = LI_WaitDisplayString;
			}
			else if (LCD_go_g) {
			 LCD_rdy_g = 0;
				LI_State = LI_Clr;
			}
			break;
		case LI_Clr:
			LI_State = LI_PositionCursor;
			LCD_WriteCmdEnd();
			i=0;
			break;
		case LI_PositionCursor:
			LI_State = LI_DisplayChar;
			LCD_WriteCmdEnd();
			break;
		case LI_DisplayChar:
			if (i<16) {
				LI_State = LI_PositionCursor;
				LCD_WriteDataEnd();
			i++;
			}
			else {
				LI_State = LI_WaitGo0;
				LCD_WriteDataEnd();
			}
			break;
		case LI_WaitGo0:
			if (!LCD_go_g) {
				LI_State = LI_WaitDisplayString;
			}
			else if (LCD_go_g) {
				LI_State = LI_WaitGo0;
			}
			break;
		default:
			LI_State = LI_Init1;
		} // Transitions

	switch(LI_State) { // State actions
		case LI_Init1:
		 LCD_rdy_g = 0;
			break;
		case LI_Init2:
			i++; // Waiting after power up
			break;
		case LI_Init3:
			LCD_WriteCmdStart(0x38);
			break;
		case LI_Init4:
			LCD_WriteCmdStart(0x06);
			break;
		case LI_Init5:
			LCD_WriteCmdStart(0x0F);
			break;
		case LI_Init6:
			LCD_WriteCmdStart(0x01); // Clear
			break;
		//////////////////////////////////////////////
		case LI_WaitDisplayString:
			LCD_rdy_g = 1;
			break;
		case LI_Clr:
			LCD_WriteCmdStart(0x01);
			break;
		case LI_PositionCursor:
			LCD_Cursor(i);			
			break;
		case LI_DisplayChar:
			LCD_WriteDataStart(LCD_string_g[i]);
			break;
		case LI_WaitGo0:
			break;
		default:
			break;
	} // State actions
}
//--------END LCD interface synchSM------------------------------------------------


// SynchSM for testing the LCD interface -- waits for button press, fills LCD with repeated random num

enum LT_States { LT_s0, LT_WaitLcdRdy, LT_WaitButton, LT_FillAndDispString,
LT_HoldGo1, LT_WaitBtnRelease } LT_State;

void LT_Tick() {
	static unsigned short j;
	static unsigned char i, x, c;
	switch(LT_State) { // Transitions
		case -1:
			LCD_counter = 74;
			LT_State = LT_s0;
			break;
		case LT_s0:
			LT_State = LT_WaitLcdRdy;
			break;
		case LT_WaitLcdRdy:
			if (!LCD_rdy_g) {
				LT_State = LT_WaitLcdRdy;
			}
			else if (LCD_rdy_g) {
				LT_State = LT_WaitButton;
			}
			break;
		case LT_WaitButton:
			if (LCD_counter < 75) {
				LT_State = LT_WaitButton;
				LCD_counter++;
			}
			else if (!(LCD_counter < 75)) { // Button active low
				LT_State = LT_FillAndDispString;
			}
			break;
		case LT_FillAndDispString:
			LT_State = LT_HoldGo1;
			break;
		case LT_HoldGo1:
			LCD_go_g=0;
		 	LT_State = LT_WaitBtnRelease;
			break;
		case LT_WaitBtnRelease:
			//if (GetBit(PINA,0)==0) { // Wait for button release
				//LT_State = LT_WaitBtnRelease;
			//}
			//else if (GetBit(PINA,0)==1) {
				LCD_counter = 0;
				LT_State = LT_WaitLcdRdy;
			//}
			break;
		default:
			LCD_counter =74;
			LT_State = LT_s0;
		} // Transitions

	switch(LT_State) { // State actions
		case LT_s0:
			LCD_go_g=0;
			strcpy(LCD_string_g, "1234567890123456"); // Init, but never seen, shows use of strcpy though
			break;
		case LT_WaitLcdRdy:
			break;
		case LT_WaitButton:
			break;
		case LT_FillAndDispString:
			++x;
			if ((x%2)==1){
			LCD_string_g[0] = 'W';
			LCD_string_g[1] = 'e';
			LCD_string_g[2] = 'l';
			LCD_string_g[3] = 'c';
			LCD_string_g[4] = 'o';
			LCD_string_g[5] = 'm';
			LCD_string_g[6] = 'e';
			LCD_string_g[7] = ' ';
			LCD_string_g[8] = 't';
			LCD_string_g[9] = 'o';
			LCD_string_g[10] = ' ';
			LCD_string_g[11] = 'S';
			LCD_string_g[12] = 'n';
			LCD_string_g[13] = 'a';
			LCD_string_g[14] = 'k';
			LCD_string_g[15] = 'e';
			LCD_string_g[16] = ' ';
			}			
			if ((x%2)==0){
			LCD_string_g[0] = 'D';
			LCD_string_g[1] = 'i';
			LCD_string_g[2] = 'f';
			LCD_string_g[3] = 'f';
			LCD_string_g[4] = 'i';
			LCD_string_g[5] = 'c';
			LCD_string_g[6] = 'u';
			LCD_string_g[7] = 'l';
			LCD_string_g[8] = 't';
			LCD_string_g[9] = 'y';
			LCD_string_g[10] = ':';
			LCD_string_g[11] = ' ';
			LCD_string_g[12] = ' ';
			LCD_string_g[13] = ' ';
			LCD_string_g[14] = ' ';
			LCD_string_g[15] = ' ';
			LCD_string_g[16] = ' ';
			}			
			LCD_go_g = 1; // Display string
			break;
		case LT_HoldGo1:
			break;
		case LT_WaitBtnRelease:
			break;
		default:
			break;
	} // State actions
}

int main(void)
{
	DDRA = 0x00; PORTA =0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	//period for the tasks
	unsigned long int LI_per = 5;
	unsigned long int LT_per = 5;
		
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(LI_per,LT_per);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCd periods for scheduler
	unsigned long int LI_period = LI_per/GCD;
	unsigned long int LT_period = LT_per/GCD;
	
	//Declare an array of tasks
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//Task 1
	task1.state = -1;
	task1.period = LI_period;
	task1.elapsedTime = LI_period;
	task1.TickFct = &LI_Tick;

	//Task 2
	task2.state = -1;
	task2.period = LT_period;
	task2.elapsedTime = LT_period;
	task2.TickFct = &LT_Tick;

	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	LI_State = -1;
	LT_State = -1;

	while(1)
	{
		for (i=0; i < numTasks; i++) 
		{
			if (tasks[i]->elapsedTime == tasks[i]->period)
			{
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 0;
}
