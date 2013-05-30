/*	SnakeGame.c - $date$
 *	Name & E-mail:  - Harrison Chen hchen030@ucr.edu
 *	CS Login: hchen030
 *	Partner(s) Name & E-mail:  -
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
unsigned char rowSnake[8];
unsigned char rowFruit[8];
unsigned char col[8];
unsigned char lose;

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
	RowRegister();
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

void ColInit()
{
	col[0] = 0x01;
	col[1] = 0x02;
	col[2] = 0x04;
	col[3] = 0x08;
	col[4] = 0x10;
	col[5] = 0x20;
	col[6] = 0x40;
	col[7] = 0x80;
}

void RowInit()
{
	for(int i = 0; i < 8; ++i)
	{
		rowSnake[i] = 0;
		rowFruit[i] = 0;
	}
}

void RowRegister()
{
	RowInit();
	ColInit();
	for(int i = 0; i < 8; ++i)
	{
		for(int j = 0; j < (snakeBody->size); ++j)
		{
			if( ((snakeBody[j].colPos)&0xFF) == ((~col[i])&0xFF) )
			{
				rowSnake[i] += (snakeBody[j].rowPos&0xFF);
			}
			if( (fruitCol&0xFF) == ((~col[i])&0xFF) )
			{
				rowFruit[i] = (fruitRow&0xFF);
			}				
		}
	}
}

int isOwnSnake()
{
	for(int i = 0; i < (snakeBody->size); ++i)
	{
		if( ((rNext&0xFF) == (snakeBody[i].rowPos&0xFF)) && ((cNext&0xFF) == (snakeBody[i].colPos&0xFF)) )
		{
			return 1;
		}
	}
	for(int i = 0; i < (snakeBody->size); ++i)
	{
		if( ((rNext&0xFF) == (snakeBody[i].rowPos&0xFF)) && (((~cNext)&0xFF) == (snakeBody[i].colPos&0xFF)) )
		{
			return 1;
		}
	}
	return 0;
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
			if(rNext == 0x00 || isOwnSnake())
			{
				LoseGame();
			}
			else if((rNext == fruitRow)&&(((snakeBody[0].colPos)&0xFF) == fruitCol))
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
			if(rNext == 0x00 || isOwnSnake())
			{
				LoseGame();
			}
			else if((rNext == fruitRow)&&(((snakeBody[0].colPos)&0xFF) == fruitCol))
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
			if(cNext == 0x00 || isOwnSnake())
			{
				LoseGame();
			}
			else if((snakeBody[0].rowPos == fruitRow)&&(((~cNext)&0xFF) == fruitCol))
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
			if(cNext == 0x00 || isOwnSnake())
			{
				LoseGame();
			}
			else if((snakeBody[0].rowPos == fruitRow)&&(((~cNext)&0xFF) == fruitCol))
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

int isSnakeThere()
{
	for(int i = 0; i < (snakeBody->size); ++i)
	{
		if( ((fruitRow&0xFF) == (snakeBody[i].rowPos&0xFF)) && ((fruitCol&0xFF) == (snakeBody[i].colPos&0xFF)) )
		{
			return 1;
		}
	}
	return 0;
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
			while(fruitGone)
			{
				fruitRow = 0x01 << (rand(change) % 8);
				fruitCol = ~(0x01 << (rand(change) % 8));
				fruitGone = isSnakeThere();
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
}

enum UpdateStates{col1, col2, col3, col4, col5, col6, col7, col8, LOST} UpdateState;

void UpdateMatrix()
{
	switch(UpdateState)
	{
		case -1:
		{
			lose = 0;
			RowRegister();
			UpdateState = col1;
			break;
		}
		case col1:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col2;
			transmit_dataB1((~col[0])&0xFF);
			transmit_dataA1(rowSnake[0]);
			transmit_dataD1(rowFruit[0]);
			break;
		}
		case col2:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col3;
			transmit_dataB1((~col[1])&0xFF);
			transmit_dataA1(rowSnake[1]);
			transmit_dataD1(rowFruit[1]);
			break;
		}
		case col3:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col4;
			transmit_dataB1((~col[2])&0xFF);
			transmit_dataA1(rowSnake[2]);
			transmit_dataD1(rowFruit[2]);
			break;
		}
		case col4:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col5;
			transmit_dataB1((~col[3])&0xFF);
			transmit_dataA1(rowSnake[3]);
			transmit_dataD1(rowFruit[3]);
			break;
		}
		case col5:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col6;
			transmit_dataB1((~col[4])&0xFF);
			transmit_dataA1(rowSnake[4]);
			transmit_dataD1(rowFruit[4]);
			break;
		}
		case col6:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col7;
			transmit_dataB1((~col[5])&0xFF);
			transmit_dataA1(rowSnake[5]);
			transmit_dataD1(rowFruit[5]);
			break;
		}
		case col7:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col8;
			transmit_dataB1((~col[6])&0xFF);
			transmit_dataA1(rowSnake[6]);
			transmit_dataD1(rowFruit[6]);
			break;
		}
		case col8:
		{
			if(lose)
			{
				UpdateState = LOST;
				break;
			}
			RowRegister();
			UpdateState = col1;
			transmit_dataB1((~col[7])&0xFF);
			transmit_dataA1(rowSnake[7]);
			transmit_dataD1(rowFruit[7]);
			break;
		}
		case LOST:
		{
			transmit_dataB1(0x00);
			transmit_dataD1(0xFF);
			if(dir == reset)
			{
				UpdateState = -1;
			}
			break;
		}
		default:
		{
			RowRegister();
			UpdateState = col1;
			break;
		}
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

void transmit_dataA2(unsigned char data) //transmit 8bits using PORTA 0 to 3
{
	int i;
	for(i = 0; i < 8; ++i)
	{
		PORTA = 0x80;
		PORTA |= ((data >> i) & 0x10); //transmit 8bits using PORTB 0 to 3
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
{
	if(keyVal == reset)
	{
		lose = 0;
	}
	else
	{
		lose = 1;
	}		
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	
	//period for the tasks
	unsigned long int Keypad_per = 50;
	unsigned long int GameOfSnakeEasy_per = 300;
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
	UpdateState = -1;
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
		
		if(keyVal == reset)
		{
			KeyState = -1;
			GameState = -1;
			UpdateState = -1;
			Fruit_Status = -1;
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 0;
}
