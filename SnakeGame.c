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

unsigned char MatrixRowPosition;
unsigned char MatrixColPosition;
unsigned char rTemp;
unsigned char cTemp;

enum dir_states{up,down,left,right} dir;

void UpdatePosition(unsigned char dir)
{
	switch(dir)
	{
		case -1:
		{
			MatrixRowPosition = 0x08;
			MatrixColPosition = 0x10;
			break;
		}
		case up:
		{
			cTemp = MatrixRowPosition << 1;
			if(cTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				MatrixRowPosition = cTemp;
			}
			break;
		}
		case down:
		{
			cTemp = MatrixColPosition >> 1;
			if(cTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				MatrixRowPosition = cTemp;
			}
			break;
		}
		case left:
		{
			cTemp = MatrixColPosition >> 1;
			if(cTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				MatrixColPosition = cTemp;
			}
			break;
		}
		case right:
		{
			cTemp = MatrixColPosition << 1;
			if(cTemp == 0x00)
			{
				LoseGame();
			}
			else
			{
				MatrixColPosition = cTemp;
			}
			break;
		}
		default:
		{
			MatrixRowPosition = 0x08;
			MatrixColPosition = 0x10;
			break;
		}
	}
}

void KeypadPress()
{
	
}

void LoseGame()
{
	
}

int main(void)
{
	while(1)
	{
		
	}
}
