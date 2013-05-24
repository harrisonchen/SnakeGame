/*  SnakeGame.c - $date$
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

void UpdateRowPos(unsigned char dir)
{
	switch(dir)
	{
		case -1:
		{
			MatrixRowPosition = 0x08;
			MatrixColPosition = 0x10;
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
		}
	}
}

void LoseGame()
{
	break;
}

int main(void)
{
	while(1)
	{
		
	}
}
