/*	SnakeGame.c - $date$
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

unsigned char matrixRowPosition;
unsigned char matrixColPosition;
unsigned char rTemp;
unsigned char cTemp;
unsigned char x, keyVal;//for keypad functions

typedef struct _Snake
{
	unsigned char rowPos;
	unsigned char colPos;
	unsigned char size;
} Snake;

Snake snakeBody[64];
Snake snakeHead;
Snake snakeTail;

unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while (1)
	{
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

enum dir_states{up,down,left,right,reset} dir;

void UpdateSnakePos()
{
	if(keyVal != -1)
	{
		dir = keyVal;	
	}	
	switch(dir)
	{
		case -1:
		{
			snakeBody->size = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			snakeTail = snakeBody[0];
			break;
		}
		case up:
		{
			rTemp = (snakeBody[0].rowPos) << 1;
			if(rTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				(snakeBody[0].rowPos) = rTemp;
			}
			break;
		}
		case down:
		{
			rTemp = (snakeBody[0].rowPos) >> 1;
			if(rTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				(snakeBody[0].rowPos) = rTemp;
			}
			break;
		}
		case left:
		{
			cTemp = (~(snakeBody[0].colPos)&0xFF) >> 1;
			if(cTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				(snakeBody[0].colPos) = ~cTemp;
			}
			break;
		}
		case right:
		{
			cTemp = (~(snakeBody[0].colPos)&0xFF) << 1;
			if(cTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				(snakeBody[0].colPos) = ~cTemp;
			}
			break;
		}
		case reset:
		{
			snakeBody->size = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			snakeTail = snakeBody[0];
			break;
		}
		default:
		{
			snakeBody->size = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			snakeTail = snakeBody[0];
			break;
		}
	}
}

void UpdateMatrix()
{
	for(int i = 0; i < (snakeBody->size); ++i)
	{
		transmit_dataB1(snakeBody[0].colPos);
		transmit_dataA1(snakeBody[0].rowPos);
	}		
}

void transmit_dataA1(unsigned char data) //transmit 8bits using PORTA 0 to 3
{
	int i;
	for(i = 0; i < 8; ++i)
	{
		PORTA = 0x08;
		PORTA |= ((data >> i) & 0x01); //transmit 8bits using PORTB 0 to 3
		PORTA |= 0x02;
	}
	PORTA |= 0x04;
	PORTA = 0x00;
}

void transmit_dataB1(unsigned char data)
{
	int i;
	for(i = 0; i < 8; ++i)
	{
		PORTB = 0x08;
		PORTB |= ((data >> i) & 0x01);
		PORTB |= 0x02;
	}
	PORTB |= 0x04;
	PORTB = 0x00;
}

enum GameStates{updateSnakePos, updateMatrix} GameState;

void GameOfSnake()
{
	switch(GameState)
	{
		case -1:
		{
			GameState = updateSnakePos;
			break;
		}
		case updateSnakePos:
		{
			GameState = updateMatrix;
			break;
		}
		case updateMatrix:
		{
			GameState = updateSnakePos;
			break;
		}
		default:
		{
			GameState = updateSnakePos;
			break;
		}
	}
	
	switch(GameState)
	{
		case updateSnakePos:
		{
			UpdateSnakePos();
			break;
		}
		case updateMatrix:
		{
			UpdateMatrix();
			break;
		}
		default:
		{
			break;
		}
	}
}

unsigned char GetKeypadKey() 
{
	PORTC = 0xEF; // Enable col 4 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('1'); }
	if (GetBit(PINC,1)==0) { return('4'); }
	if (GetBit(PINC,2)==0) { return('7'); }
	if (GetBit(PINC,3)==0) { return('*'); }

	// Check keys in col 2
	PORTC = 0xDF; // Enable col 5 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('2'); }
	if (GetBit(PINC,1)==0) { return('5'); }
	if (GetBit(PINC,2)==0) { return('8'); }
	if (GetBit(PINC,3)==0) { return('0'); }
	// ... *****FINISH*****

	// Check keys in col 3
	PORTC = 0xBF; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('3'); }
	if (GetBit(PINC,1)==0) { return('6'); }
	if (GetBit(PINC,2)==0) { return('9'); }
	if (GetBit(PINC,3)==0) { return('#'); }
	// ... *****FINISH*****

	// Check keys in col 4	
	PORTC = 0x7F; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('A'); }
	if (GetBit(PINC,1)==0) { return('B'); }
	if (GetBit(PINC,2)==0) { return('C'); }
	if (GetBit(PINC,3)==0) { return('D'); }
	// ... *****FINISH*****

	return('\0'); // default value

}

enum KeyStates {Wait, Pressed, Released} KeyState;

void KeyOut(unsigned char in)
{
	switch (in)
	{
		case '\0': keyVal = -1; break;
		case '1': keyVal = -1; break;
		case '2': keyVal = right; break;
		case '3': keyVal = -1; break;
		case '4': keyVal = up; break;
		case '5': keyVal = down; break;
		case '6': keyVal = -1; break;
		case '7': keyVal = -1; break;
		case '8': keyVal = left; break;
		case '9': keyVal = -1; break;
		case 'A': keyVal = -1; break;
		case 'B': keyVal = -1; break;
		case 'C': keyVal = -1; break;
		case 'D': keyVal = reset; break;
		case '*': keyVal = -1; break;
		case '0': keyVal = -1; break;
		case '#': keyVal = -1; break;
		default: keyVal = -1; break;
	}
}

void GetKeyPress()
{
	switch(KeyState)
	{
		case -1:
		{
			keyVal = -1;
			KeyState = Wait;
			break;
		}
		case Wait:
		{
			x = GetKeypadKey();
			if(x != '\0')
			{
				KeyOut(x);
				KeyState = Pressed;
				//PORTA = keyVal;
			}
			break;
		}
		case Pressed:
		{
			x = GetKeypadKey();
			if(x == '\0')
			{
				KeyState = Wait;
			}
			break;
		}
		default:
		{
			keyVal = -1;
			KeyState = Wait;
			break;
		}
	}
}

void LoseGame()
{
	
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	
	//period for the tasks
	unsigned long int Keypad_per = 50;
	unsigned long int GameOfSnake_per = 200;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(Keypad_per,1);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCd periods for scheduler
	unsigned long int Keypad_period = Keypad_per/GCD;
	unsigned long int GameOfSnake_period = GameOfSnake_per/GCD;
	
	//Declare an array of tasks
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//Task 1
	task1.state = -1;
	task1.period = Keypad_period;
	task1.elapsedTime = Keypad_period;
	task1.TickFct = &GetKeyPress;

	//Task 2
	task2.state = -1;
	task2.period = GameOfSnake_period;
	task2.elapsedTime = GameOfSnake_period;
	task2.TickFct = &GameOfSnake;

	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	KeyState = -1;
	GameState = -1;

	while(1)
	{
		for (i=0; i < numTasks; i++) 
		{
			if (tasks[i]->elapsedTime == tasks[i]->period)
			{
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 0;
}
