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

//MICROCONTROLLER COMMUNICATION SIGNALS
unsigned char TransferData;
unsigned char ResetMC = 0x1; //reset game
unsigned char StartMC = 0x2; //starts game
unsigned char EasyMC = 0x3; //easy mode chosen
unsigned char NormalMC = 0x4; //normal mode chosen
unsigned char HardMC = 0x5; //hard mode chosen
unsigned char IncrementMC = 0x6; //increment score count
unsigned char LoseMC = 0x7; //lose game
////////////////////////////////////////

unsigned char easyEn; //bool for easy mode enabling
unsigned char normalEn; //bool for normal mode enabling
unsigned char hardEn; //bool for hard mode enabling
unsigned char matrixRowPosition;
unsigned char matrixColPosition;
unsigned char rNext; //next row position
unsigned char cNext; //next colomn position
unsigned char x, keyVal;//for keypad functions
unsigned char fruitGone; //bool for whether fruit was ate by snake
unsigned char fruitRow; //row position for fruit
unsigned char fruitCol; //col position for fruit
unsigned char change;//seed for random function. it gets incremented.
unsigned char rowSnake[8]; //array storage of all row positions of snake corresponding to column
unsigned char rowFruit[8]; //array storage of all row positions of fruit corresponding to column
unsigned char col[8]; //holds column values

unsigned short dataSnakeFruit; //array storage for outputting both snake and fruit row positions combined
unsigned short block1 = (0x40 + 0x20); //hard mode block row positions
unsigned short block2 = (0x04 + 0x02); //second block
unsigned char lose; //bool for lose
unsigned char win; //bool for win

///////////////////////////////// Speaker Controller //////////////////////////////////////
enum Button_States {WaitSpeaker, PressedSpeaker, HeldSpeaker} Button_State;

void Speaker_Func()
{
	switch(Button_State)
	{
		case -1:
		{
			Button_State = WaitSpeaker;
			break;
		}
		case WaitSpeaker:
		{
			if((PIND&0x80) == 0x80)
			{
				Button_State = PressedSpeaker;
			}
			break;
		}
		case PressedSpeaker:
		{
			if((PIND&0x80) == 0x00)
			{
				Button_State = WaitSpeaker;
			}
			else
			{
				Button_State = HeldSpeaker;
			}
			break;
		}
		case HeldSpeaker:
		{
			if((PIND&0x80) == 0x00)
			{
				Button_State = WaitSpeaker;
			}
			else
			{
				Button_State = PressedSpeaker;
			}
			break;
		}
		default:
		{
			Button_State = WaitSpeaker;
			break;
		}
	}
	
	switch(Button_State)
	{
		case WaitSpeaker:
		{
			break;
		}
		case PressedSpeaker:
		{
			PORTD = (PIND|0x40);
			break;
		}
		case HeldSpeaker:
		{
			PORTD = (PIND&0xBF);
			break;
		}
		default:
		{
			break;
		}
	}
}

/////////////////////////////// KeyPad Multiplexer ///////////////////////////////////
enum dir_states{up,down,left,right,reset} dir;//direction states
	
enum gameTaskStates{t1,t2,t3,t4,WaitGame,Start,Easy,Normal,Hard,Lose,Win,Clear}gameTaskState;//game states

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
	PORTC = 0x7F; // Enable col 7 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('A'); }
	if (GetBit(PINC,1)==0) { return('B'); }
	if (GetBit(PINC,2)==0) { return('C'); }
	if (GetBit(PINC,3)==0) { return('D'); }
	// ... *****FINISH*****

	return('\0'); // default value

}

/////////////////////////////// KeyPad Decoder ////////////////////////////////////
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
		case 'A': keyVal = Hard; break;
		case 'B': keyVal = Normal; break;
		case 'C': keyVal = Easy; break;
		case 'D': keyVal = Start; break;
		case '*': keyVal = reset; break;
		case '0': keyVal = -1; break;
		case '#': keyVal = -1; break;
		default: keyVal = -1; break;
	}
}

/////////////////////////////// KeyPad Controller ////////////////////////////////
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

////////////////////////////// Snake Structure //////////////////////////////////
typedef struct _Snake
{
	unsigned char rowPos;
	unsigned char colPos;
	unsigned char size;
} Snake;

Snake snakeBody[64]; //array for all snake positions both row and column
Snake snakeBodyTemp[64]; //for array adjustment function copy
Snake snakeHead; //head of snake positon
Snake snakeTail; //tail of snake position
Snake snakeHold; //for shifting snake position array
 
/////////////////////////// GCD Function //////////////////////////////////
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

//////////////////////////// Task Structure ////////////////////////////////
typedef struct _task {
	signed char state; //current state
	unsigned long int period; //time period
	unsigned long int elapsedTime; //time passed since last period
	int (*TickFct) (int);
} task;

//////////////////////////// Snake Position Adjustment Function/////////////////////
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

///////////////////////////// Shift Snake Row Function ///////////////////////////
void SnakeShiftY()
{
	snakeHead.rowPos = rNext;
	SnakeArrayAdj();
}

//////////////////////////// Shift Snake Col Function ////////////////////////////
void SnakeShiftX()
{
	snakeHead.colPos = cNext;
	SnakeArrayAdj();
}

//////////////////////////// Add Snake Head Function and Score ///////////////////
void SnakeShiftGrowY()//When snake eats apple up or down
{
	(snakeBody->size)++;
	snakeHead.rowPos = rNext;
	SnakeArrayAdj();
	//ScoreBoard();
	TransferData = IncrementMC;
	PORTD = ((PIND&0x80) + TransferData);
}

//////////////////////////// Add Snake Head Function and Score ///////////////////
void SnakeShiftGrowX()//When snake eats apple left or right
{
	(snakeBody->size)++;
	snakeHead.colPos = cNext;
	SnakeArrayAdj();
	//ScoreBoard();
	TransferData = IncrementMC;
	PORTD = ((PIND&0x80) + TransferData);
}

///////////////////////// Column Value Initializer ////////////////////////////
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

//////////////////////////// Row Value Initializer ////////////////////////////
void RowInit()
{
	for(int i = 0; i < 8; ++i)
	{
		rowSnake[i] = 0;
		rowFruit[i] = 0;
	}
}

/////// Storage for all row positions for both snake and apple /////////
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

//////////// Check Function for if next position is the Snake itself //////////////
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

//////////////////// Game Task Manager //////////////////////
void GameTask()
{
	switch(gameTaskState)
	{
		case -1:
		{
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			easyEn = 0;
			normalEn = 0;
			hardEn = 0;
			gameTaskState = WaitGame;
			break;
		}
		case WaitGame:
		{
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			easyEn = 0;
			normalEn = 0;
			hardEn = 0;
			if(keyVal == Start)
			{
				gameTaskState = Start;
				TransferData = StartMC;
				PORTD = ((PIND&0x80) + TransferData);
			}				
			break;
		}
		case Start:
		{
			//TransferData = 0x0;
			//PORTD = TransferData;
			if(keyVal == Easy)
			{
				gameTaskState = Easy;
				easyEn = 1;
				TransferData = EasyMC;
				PORTD = ((PIND&0x80) + TransferData);
			}
			else if(keyVal == Normal)
			{
				gameTaskState = Normal;
				normalEn = 1;
				TransferData = NormalMC;
				PORTD = ((PIND&0x80) + TransferData);
			}
			else if(keyVal == Hard)
			{
				gameTaskState = Hard;
				hardEn = 1;
				TransferData = HardMC;
				PORTD = ((PIND&0x80) + TransferData);
			}
			else if(keyVal == reset)
			{
				gameTaskState = Clear;
			}
			break;
		}
		case Easy:
		{
			PORTD = (PIND&0x3F);
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			if(lose)
			{
				gameTaskState = Lose;
			}
			else if(keyVal == reset)
			{
				gameTaskState = Clear;
			}
			break;
		}
		case Normal:
		{
			PORTD = (PIND&0x3F);
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			if(lose)
			{
				gameTaskState = Lose;
			}
			else if(keyVal == reset)
			{
				gameTaskState = Clear;
			}
			break;
		}
		case Hard:
		{
			PORTD = (PIND&0x3F);
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			if(lose)
			{
				gameTaskState = Lose;
			}
			else if(keyVal == reset)
			{
				gameTaskState = Clear;
			}
			break;
		}
		case Lose:
		{
			PORTD = (PIND|0x80);
			TransferData = LoseMC;
			PORTD = ((PIND&0x80) + TransferData);
			easyEn = 0;
			normalEn = 0;
			hardEn = 0;
			if(keyVal == reset)
			{
				gameTaskState = Clear;
			}
			break;
		}
		case Win:
		{
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			easyEn = 0;
			normalEn = 0;
			hardEn = 0;
			if(keyVal == Clear)
			{
				gameTaskState = Clear;
			}
			break;
		}
		case Clear:
		{
			ResetGame();
			gameTaskState = WaitGame;
			break;
		}
		default:
		{
			TransferData = 0x0;
			PORTD = ((PIND&0x80) + TransferData);
			easyEn = 0;
			normalEn = 0;
			hardEn = 0;
			gameTaskState = WaitGame;
			break;
		}
	}		
}

//////////////////////// Update Snake Manager ///////////////////////////
void UpdateSnakePos()
{
	if((keyVal == up) ||(keyVal == down) ||(keyVal == left) ||(keyVal == right))
	{
		dir = keyVal;
	}
	switch(dir)
	{
		case up:
		{
			rNext = (snakeBody[0].rowPos) << 1;
			if(rNext == 0x00 || isOwnSnake())
			{
				lose = 1;
			}
			else if(hardEn && (rNext == 0x20) && ( ((~(snakeBody[0].colPos)&0xFF) == 0x02)||(((~snakeBody[0].colPos)&0xFF) == 0x04)))
			{
				lose = 1;
			}
			else if(hardEn && (rNext == 0x02) && ( ((~(snakeBody[0].colPos)&0xFF) == 0x20)||(((~snakeBody[0].colPos)&0xFF) == 0x40)))
			{
				lose = 1;
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
				lose = 1;
			}
			else if(hardEn && (rNext == 0x40) && ( ((~(snakeBody[0].colPos)&0xFF) == 0x02)||(((~snakeBody[0].colPos)&0xFF) == 0x04)))
			{
				lose = 1;
			}
			else if(hardEn && (rNext == 0x04) && ( ((~(snakeBody[0].colPos)&0xFF) == 0x20)||(((~snakeBody[0].colPos)&0xFF) == 0x40)))
			{
				lose = 1;
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
				lose = 1;
			}
			else if(hardEn && (cNext == 0x04) && ( (snakeBody[0].rowPos == 0x40)||(snakeBody[0].rowPos == 0x20)))
			{
				lose = 1;
			}
			else if(hardEn && (cNext == 0x40) && ( (snakeBody[0].rowPos == 0x04)||(snakeBody[0].rowPos == 0x02)))
			{
				lose = 1;
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
				lose = 1;
			}
			else if(hardEn && (cNext == 0x02) && ( (snakeBody[0].rowPos == 0x40)||(snakeBody[0].rowPos == 0x20)))
			{
				lose = 1;
			}
			else if(hardEn && (cNext == 0x20) && ( (snakeBody[0].rowPos == 0x04)||(snakeBody[0].rowPos == 0x02)))
			{
				lose = 1;
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
			lose = 0;
			(snakeBody->size) = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			break;
		}
		default:
		{
			lose = 0;
			(snakeBody->size) = 1;
			(snakeBody[0].rowPos) = 0x08;
			(snakeBody[0].colPos) = ~(0x10);
			snakeHead = snakeBody[0];
			break;
		}
	}
}

////////////////////// Bool Function to check if new fruit position is on snake ///////////////////////
int isSnakeThere()
{
	for(int i = 0; i < (snakeBody->size); ++i)
	{
		if( ((fruitRow&0xFF) == (snakeBody[i].rowPos&0xFF)) && ((fruitCol&0xFF) == (snakeBody[i].colPos&0xFF)) )
		{
			return 1;
		}
		else if( hardEn && (((fruitRow&0xFF)==(0x40))||((fruitRow&0xFF)==(0x20))) && (((~fruitCol)&0xFF)==(0x02)))
		{
			return 1;
		}
		else if( hardEn && (((fruitRow&0xFF)==(0x40))||((fruitRow&0xFF)==(0x20))) && (((~fruitCol)&0xFF)==(0x04)))
		{
			return 1;
		}
		else if( hardEn && (((fruitRow&0xFF)==(0x04))||((fruitRow&0xFF)==(0x02))) && (((~fruitCol)&0xFF)==(0x20)))
		{
			return 1;
		}
		else if( hardEn && (((fruitRow&0xFF)==(0x04))||((fruitRow&0xFF)==(0x02))) && (((~fruitCol)&0xFF)==(0x40)))
		{
			return 1;
		}
	}
	return 0;
}

////////////////////////////// Fruit Position Controller ///////////////////////////////////////
enum Fruit_States{fresh,devoured} Fruit_Status;

void GenerateFruit()
{
	switch(Fruit_Status)
	{
		case -1:
		{
			change = 1;
			fruitGone = 1;
			while(fruitGone)
			{
				change++;
				fruitRow = 0x01 << (rand(change) % 8);
				fruitCol = ~(0x01 << (rand(change) % 8));
				fruitGone = isSnakeThere();
			}
			//fruitGone = 0;
			Fruit_Status = fresh;
			break;
		}
		case fresh:
		{
			if(fruitGone)
			{
				Fruit_Status = devoured;
				PORTD = (PIND|0x80);
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
			fruitGone = 1;
			while(fruitGone)
			{
				change++;
				fruitRow = 0x01 << (rand(change) % 8);
				fruitCol = ~(0x01 << (rand(change) % 8));
				fruitGone = isSnakeThere();
			}
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
				change++;
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

//////////////////////// Matrix Update Controller //////////////////////////////////
enum UpdateStates{col1, col2, col3, col4, col5, col6, col7, col8, LOST,wait1,wait2,wait3,wait4,wait5,wait6,wait7,wait8,} UpdateState;

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
				RowRegister();
				UpdateState = wait1;
				dataSnakeFruit = rowSnake[0];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[0]);
				transmit_dataB1((~col[0])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait1;
				dataSnakeFruit = rowSnake[0];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[0]);
				transmit_dataB1((~col[0])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[0]);
			}
			break;		
		}
		case wait1:
		{
			UpdateState = col2;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col2:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait2;
				dataSnakeFruit = rowSnake[1];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[1]);
				transmit_dataB1((~col[1])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait2;
				dataSnakeFruit = rowSnake[1];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[1]);
				if(gameTaskState == Hard)
				{
					dataSnakeFruit = dataSnakeFruit + (block1);
				}
				transmit_dataB1((~col[1])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[1]);
			}				
			break;
		}
		case wait2:
		{
			UpdateState = col3;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col3:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait3;
				dataSnakeFruit = rowSnake[2];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[2]);
				transmit_dataB1((~col[2])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait3;
				dataSnakeFruit = rowSnake[2];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[2]);
				if(gameTaskState == Hard)
				{
					dataSnakeFruit = dataSnakeFruit + (block1);
				}
				transmit_dataB1((~col[2])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[2]);
			}				
			break;
		}
		case wait3:
		{
			UpdateState = col4;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col4:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait4;
				dataSnakeFruit = rowSnake[3];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[3]);
				transmit_dataB1((~col[3])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait4;
				dataSnakeFruit = rowSnake[3];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[3]);
				transmit_dataB1((~col[3])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[3]);
			}				
			break;
		}
		case wait4:
		{
			UpdateState = col5;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col5:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait5;
				dataSnakeFruit = rowSnake[4];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[4]);
				transmit_dataB1((~col[4])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait5;
				dataSnakeFruit = rowSnake[4];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[4]);
				transmit_dataB1((~col[4])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[4]);
			}				
			break;
		}
		case wait5:
		{
			UpdateState = col6;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col6:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait6;
				dataSnakeFruit = rowSnake[5];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[5]);
				transmit_dataB1((~col[5])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait6;
				dataSnakeFruit = rowSnake[5];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[5]);
				if(gameTaskState == Hard)
				{
					dataSnakeFruit = dataSnakeFruit + (block2);
				}
				transmit_dataB1((~col[5])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[5]);
			}				
			break;
		}
		case wait6:
		{
			UpdateState = col7;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col7:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait7;
				dataSnakeFruit = rowSnake[6];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[6]);
				transmit_dataB1((~col[6])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait7;
				dataSnakeFruit = rowSnake[6];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[6]);
				if(gameTaskState == Hard)
				{
					dataSnakeFruit = dataSnakeFruit + (block2);
				}
				transmit_dataB1((~col[6])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[6]);
			}				
			break;
		}
		case wait7:
		{
			UpdateState = col8;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
			break;
		}
		case col8:
		{
			if(lose)
			{
				RowRegister();
				UpdateState = wait8;
				dataSnakeFruit = rowSnake[7];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowSnake[7]);
				transmit_dataB1((~col[7])&0xFF);
				transmit_dataA1(dataSnakeFruit);
			}
			else if(easyEn || normalEn || hardEn)
			{
				RowRegister();
				UpdateState = wait8;
				dataSnakeFruit = rowSnake[7];
				dataSnakeFruit = ((dataSnakeFruit << 8) + rowFruit[7]);
				transmit_dataB1((~col[7])&0xFF);
				transmit_dataA1(dataSnakeFruit);
				//transmit_dataD1(rowFruit[7]);
			}				
			break;
		}
		case wait8:
		{
			UpdateState = col1;
			transmit_dataB1(0xFF);
			transmit_dataA1(0x0000);
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

/////////////// Send Data to Shift Register on Port A /////////////////
void transmit_dataA1(unsigned short data) //transmit 8bits using PORTA 0 to 3
{
	int i;
	for(i = 0; i < 16; ++i)
	{
		PORTA = 0x08;
		PORTA |= ((data >> i) & 0x01); //transmit 8bits using PORTB 0 to 3
		PORTA |= 0x02;
	}
	PORTA |= 0x04;
	PORTA = 0x00;
}

/////////////// Send Data to Shift Register on Port B ////////////////
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

///////////////////// Easy Mode Controller/////////////////////
enum GameStatesEasy{updateSnakePosEasy} GameStateEasy;

void GameOfSnakeEasy()
{
	switch(GameStateEasy)
	{
		case -1:
		{
			if(easyEn == 1)
			{
				GameStateEasy = updateSnakePosEasy;
				(snakeBody->size) = 1;
				(snakeBody[0].rowPos) = 0x08;
				(snakeBody[0].colPos) = ~(0x10);
				snakeHead = snakeBody[0];
			}				
			break;
		}
		case updateSnakePosEasy:
		{
			break;
		}
		default:
		{
			if(easyEn == 1)
			{
				GameStateEasy = updateSnakePosEasy;
				(snakeBody->size) = 1;
				(snakeBody[0].rowPos) = 0x08;
				(snakeBody[0].colPos) = ~(0x10);
				snakeHead = snakeBody[0];
			}
			break;
		}
	}
	
	switch(GameStateEasy)
	{
		case updateSnakePosEasy:
		{
			if(easyEn)
			{
				UpdateSnakePos();
			}				
			break;
		}
		default:
		{
			break;
		}
	}
}

///////////////////// Normal Mode Controller/////////////////////
enum GameStatesNormal{updateSnakePosNormal} GameStateNormal;

void GameOfSnakeNormal()
{
	switch(GameStateNormal)
	{
		case -1:
		{
			if(normalEn == 1)
			{
				GameStateNormal = updateSnakePosNormal;
				(snakeBody->size) = 1;
				(snakeBody[0].rowPos) = 0x08;
				(snakeBody[0].colPos) = ~(0x10);
				snakeHead = snakeBody[0];
			}
			break;
		}
		case updateSnakePosNormal:
		{
			break;
		}
		default:
		{
			if(normalEn == 1)
			{
				GameStateNormal = updateSnakePosNormal;
				(snakeBody->size) = 1;
				(snakeBody[0].rowPos) = 0x08;
				(snakeBody[0].colPos) = ~(0x10);
				snakeHead = snakeBody[0];
			}
			break;
		}
	}
	
	switch(GameStateNormal)
	{
		case updateSnakePosNormal:
		{
			if(normalEn)
			{
				UpdateSnakePos();
			}				
			break;
		}
		default:
		{
			break;
		}
	}
}

///////////////////// Hard Mode Controller/////////////////////
enum GameStatesHard{updateSnakePosHard} GameStateHard;

void GameOfSnakeHard()
{
	switch(GameStateHard)
	{
		case -1:
		{
			if(hardEn == 1)
			{
				GameStateHard = updateSnakePosHard;
				(snakeBody->size) = 1;
				(snakeBody[0].rowPos) = 0x08;
				(snakeBody[0].colPos) = ~(0x10);
				snakeHead = snakeBody[0];
			}
			break;
		}
		case updateSnakePosHard:
		{
			break;
		}
		default:
		{
			if(hardEn == 1)
			{
				GameStateHard = updateSnakePosHard;
				(snakeBody->size) = 1;
				(snakeBody[0].rowPos) = 0x08;
				(snakeBody[0].colPos) = ~(0x10);
				snakeHead = snakeBody[0];
			}
			break;
		}
	}
	
	switch(GameStateHard)
	{
		case updateSnakePosHard:
		{
			if(hardEn)
			{
				UpdateSnakePos();
			}				
			break;
		}
		default:
		{
			break;
		}
	}
}

/////////////////// Reset Game Function ////////////////////
void ResetGame()
{
	PORTD = (PIND&0x3F);
	TransferData = ResetMC;
	PORTD = ((PIND&0x80) + TransferData);
	//gameTaskState = -1;
	KeyState = -1;
	GameStateEasy = -1;
	GameStateNormal = -1;
	GameStateHard = -1;
	UpdateState = -1;
	Fruit_Status = -1;
	dir = -1;
	//score = 0;
	lose = 0;
	
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	
	//period for the tasks
	unsigned long int GameTask_per = 50;
	unsigned long int Keypad_per = 50;
	unsigned long int GameOfSnakeEasy_per = 400;
	unsigned long int UpdateMatrix_per = 1;
	unsigned long int GenerateFruit_per = 50;
	unsigned long int GameOfSnakeNormal_per = 200;
	unsigned long int GameOfSnakeHard_per = 200;
	unsigned long int SpeakerTick_per = 3;
	
	//Calculating GCD
	unsigned long int tmpGCD;
	tmpGCD = findGCD(Keypad_per,GameOfSnakeEasy_per);
	tmpGCD = findGCD(tmpGCD,UpdateMatrix_per);
	tmpGCD = findGCD(tmpGCD,GenerateFruit_per);
	tmpGCD = findGCD(tmpGCD,GameTask_per);
	tmpGCD = findGCD(tmpGCD,GameOfSnakeNormal_per);
	tmpGCD = findGCD(tmpGCD,GameOfSnakeHard_per);
	
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCd periods for scheduler
	unsigned long int GameTask_period = GameTask_per/GCD;
	unsigned long int Keypad_period = Keypad_per/GCD;
	unsigned long int GameOfSnakeEasy_period = GameOfSnakeEasy_per/GCD;
	unsigned long int UpdateMatrix_period = UpdateMatrix_per/GCD;
	unsigned long int GenerateFruit_period = GenerateFruit_per/GCD;
	unsigned long int GameOfSnakeNormal_period = GameOfSnakeNormal_per/GCD;
	unsigned long int GameOfSnakeHard_period = GameOfSnakeHard_per/GCD;
	long double SpeakerTick_period = SpeakerTick_per/GCD;
	
	//Declare an array of tasks
	static task task0, task1, task2, task3, task4, task5, task6, task7;
	task *tasks[] = {&task0, &task1, &task2, &task3, &task4, &task5, &task6, &task7};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//Task 0
	task0.state = -1;
	task0.period = GameTask_period;
	task0.elapsedTime = GameTask_period;
	task0.TickFct = &GameTask;

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
	
	//Task 5
	task5.state = -1;
	task5.period = GameOfSnakeNormal_period;
	task5.elapsedTime = GameOfSnakeNormal_period;
	task5.TickFct = &GameOfSnakeNormal;
	
	//Task 6
	task6.state = -1;
	task6.period = GameOfSnakeHard_period;
	task6.elapsedTime = GameOfSnakeHard_period;
	task6.TickFct = &GameOfSnakeHard;
	
	//Task 7
	task7.state = -1;
	task7.period = SpeakerTick_period;
	task7.elapsedTime = SpeakerTick_period;
	task7.TickFct = &Speaker_Func;

	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	gameTaskState = -1;
	KeyState = -1;
	GameStateEasy = -1;
	GameStateNormal = -1;
	GameStateHard = -1;
	UpdateState = -1;
	Fruit_Status = -1;
	dir = -1;

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