/*	SnakeGame.c - $date$
 *	Name & E-mail:  - Harrison Chen hchen030@ucr.edu
 *	CS Login: hchen030
 *	Partner(s) Name & E-mail:  -  Patrick Ly-Vo plyvo001@ucr.edu
 *	Lab Section: 22
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
unsigned char rNext;
unsigned char cNext;
unsigned char x, keyVal;//for keypad functions
unsigned char fruitGone;
unsigned char fruitRow;
unsigned char fruitCol;
unsigned char change;//seed for random function. it gets incremented.


typedef struct _Snake
{
	unsigned char rowPos;
	unsigned char colPos;
	unsigned char size;
} Snake;

Snake snakeBody[64];
Snake snakeBodyTemp[64]; //for array adjustment function copy
Snake snakeHead;
Snake snakeTail;
Snake snakeHold; //for shifting snake position array

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

void SnakeArrayAdj()
{
	(snakeBodyTemp->size) = 1;
	(snakeBodyTemp[0].rowPos) = (snakeHead.rowPos);
	(snakeBodyTemp[0].colPos) = (snakeHead.colPos);
	for(int i = 1; i < (snakeBody->size); ++i)
	{
		snakeBodyTemp[i].rowPos = snakeBody[i - 1].rowPos;
		snakeBodyTemp[i].colPos = snakeBody[i - 1].colPos;
		(snakeBodyTemp->size) += 1;
	}
	(snakeBody->size) = 0;
	for(int j = 0; j < (snakeBodyTemp->size); ++j)
	{
		snakeBody[j].rowPos = snakeBodyTemp[j].rowPos;
		snakeBody[j].colPos = snakeBodyTemp[j].colPos;
		(snakeBody->size) += 1;
	}
}

void SnakeShiftY()
{
	snakeHead.rowPos = rNext;
	SnakeArrayAdj();
}

void SnakeShiftX()
{
	snakeHead.colPos = cNext;
	SnakeArrayAdj();
}

void SnakeShiftGrowY()
{
	(snakeBody->size)++;
	snakeHead.rowPos = rNext;
	SnakeArrayAdj();
}

void SnakeShiftGrowX()
{
	(snakeBody->size)++;
	snakeHead.colPos = cNext;
	SnakeArrayAdj();
}

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
			(snakeBody->size) = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			break;
		}
		case up:
		{
			rNext = (snakeBody[0].rowPos) << 1;
			if(rNext == 0x00)
			{
				LoseGame();
			}
			else if((rNext == fruitRow)&&((snakeBody[0].colPos)&0xFF) == fruitCol)
			{
				fruitGone = 1;
				SnakeShiftGrowY();
			}
			else
			{
				SnakeShiftY();
			}
			break;
		}
		case down:
		{
			rNext = (snakeBody[0].rowPos) >> 1;
			if(rNext == 0x00)
			{
				LoseGame();
			}
			else if((rNext == fruitRow)&&((snakeBody[0].colPos)&0xFF) == fruitCol)
			{
				fruitGone = 1;
				SnakeShiftGrowY();
			}
			else
			{
				SnakeShiftY();
			}
			break;
		}
		case left:
		{
			cNext = (~(snakeBody[0].colPos)&0xFF) >> 1;
			if(cNext == 0x00)
			{
				LoseGame();
			}
			else if((rNext == fruitRow)&&(~cNext == fruitCol))
			{
				fruitGone = 1;
				cNext = ~cNext;
				SnakeShiftGrowX();
			}
			else
			{
				cNext = ~cNext;
				SnakeShiftX();
			}
			break;
		}
		case right:
		{
			cNext = (~(snakeBody[0].colPos)&0xFF) << 1;
			if(cNext == 0x00)
			{
				LoseGame();
			}
			else if((rNext == fruitRow)&&(~cNext == fruitCol))
			{
				fruitGone = 1;
				cNext = ~cNext;
				SnakeShiftGrowX();
			}
			else
			{
				cNext = ~cNext;
				SnakeShiftX();
			}
			break;
		}
		case reset:
		{
			(snakeBody->size) = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			break;
		}
		default:
		{
			(snakeBody->size) = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			break;
		}
	}
}

enum Fruit_States{fresh,devoured} Fruit_Status;

void GenerateFruit()
{
	switch(Fruit_Status)
	{
		case -1:
		{
			change = 1;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			fruitGone = 0;
			Fruit_Status = fresh;
			break;
		}
		case fresh:
		{
			if(fruitGone)
			{
				Fruit_Status = devoured;
			}
			break;
		}
		case devoured:
		{
			if(!fruitGone)
			{
				Fruit_Status = fresh;
			}
			break;
		}
		default:
		{
			change = 1;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			fruitGone = 0;
			Fruit_Status = fresh;
			break;
		}
	}
	
	switch(Fruit_Status)
	{
		case fresh:
		{
			break;
		}
		case devoured:
		{
			change++;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			fruitGone = 0;
			break;
		}
		default:
		{
			change = 1;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			fruitGone = 0;
			Fruit_Status = fresh;
			break;
		}
	}
}

void UpdateMatrix()
{
	for(int i = 0; i < (snakeBody->size); ++i)
	{
		transmit_dataB1(snakeBody[i].colPos);
		transmit_dataA1(snakeBody[i].rowPos);
	}
	transmit_dataB1(fruitCol);
	transmit_dataD1(fruitRow);
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

void transmit_dataA2(unsigned char data) //transmit 8bits using PORTA 0 to 3
{
	int i;
	for(i = 0; i < 8; ++i)
	{
		PORTA = 0x80;
		PORTA |= ((data >> i) & 0x01); //transmit 8bits using PORTB 0 to 3
		PORTA |= 0x20;
	}
	PORTA |= 0x40;
	PORTA = 0x00;
}

void transmit_dataD1(unsigned char data) //transmit 8bits using PORTA 0 to 3
{
	int i;
	for(i = 0; i < 8; ++i)
	{
		PORTD = 0x08;
		PORTD |= ((data >> i) & 0x01); //transmit 8bits using PORTB 0 to 3
		PORTD |= 0x02;
	}
	PORTD |= 0x04;
	PORTD = 0x00;
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

void GameOfSnakeEasy()
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
			//GameState = updateMatrix;
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
			//UpdateMatrix();
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
{/*
	transmit_dataA1(0xFF);
	transmit_dataB1(~(0xFF));
	switch(dir)
	{
		case up:
		{
			
			break;
		}
		case down:
		{
			break;
		}
		case left:
		{
			break;
		}
		case right:
		{
			break;
		}
		default:
		{
			break;
		}
	}*/
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	
	//period for the tasks
	unsigned long int Keypad_per = 50;
	unsigned long int GameOfSnakeEasy_per = 150;
	unsigned long int UpdateMatrix_per = 1;
	unsigned long int GenerateFruit_per = 50;
	
	//Calculating GCD
	unsigned long int tmpGCD;
	tmpGCD = findGCD(Keypad_per,GameOfSnakeEasy_per);
	tmpGCD = findGCD(tmpGCD,UpdateMatrix_per);
	tmpGCD = findGCD(tmpGCD,GenerateFruit_per);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCd periods for scheduler
	unsigned long int Keypad_period = Keypad_per/GCD;
	unsigned long int GameOfSnakeEasy_period = GameOfSnakeEasy_per/GCD;
	unsigned long int UpdateMatrix_period = UpdateMatrix_per/GCD;
	unsigned long int GenerateFruit_period = GenerateFruit_per/GCD;
	
	//Declare an array of tasks
	static task task1, task2, task3, task4;
	task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//Task 1
	task1.state = -1;
	task1.period = Keypad_period;
	task1.elapsedTime = Keypad_period;
	task1.TickFct = &GetKeyPress;

	//Task 2
	task2.state = -1;
	task2.period = GameOfSnakeEasy_period;
	task2.elapsedTime = GameOfSnakeEasy_period;
	task2.TickFct = &GameOfSnakeEasy;

	//Task 3
	task3.state = -1;
	task3.period = UpdateMatrix_period;
	task3.elapsedTime = UpdateMatrix_period;
	task3.TickFct = &UpdateMatrix;
	
	//Task 4
	task4.state = -1;
	task4.period = UpdateMatrix_period;
	task4.elapsedTime = UpdateMatrix_period;
	task4.TickFct = &GenerateFruit;

	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	KeyState = -1;
	GameState = -1;
	Fruit_Status = -1;

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
