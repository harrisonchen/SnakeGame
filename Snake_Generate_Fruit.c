/*  Snake_Generate_Fruit.c - $date$
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <ucr/bit.h>
#include <ucr/timer.h>
#include <stdio.h>
//need time.h

//srand(time(0));
unsigned char fruitGone;
unsigned char fruitRow;
unsigned char fruitCol;
unsigned char change;

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

enum Fruit_States{fresh,devoured} Fruit_Status;

void GenerateFruit()
{
	switch(Fruit_Status)
	{
		case -1:
		{
			change = 0;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			transmit_dataA1(fruitRow);
			transmit_dataB1(fruitCol);
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
			change = 0;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			transmit_dataA1(fruitRow);
			transmit_dataB1(fruitCol);
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
			transmit_dataA1(fruitRow);
			transmit_dataB1(fruitCol);
			fruitGone = 0;
			break;
		}
		default:
		{
			change = 0;
			fruitRow = 0x01 << (rand(change) % 8);
			fruitCol = ~(0x01 << (rand(change) % 8));
			transmit_dataA1(fruitRow);
			transmit_dataB1(fruitCol);
			fruitGone = 0;
			Fruit_Status = fresh;
			break;
		}
	}
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	Fruit_Status = -1;
	
	while(1)
	{
		
		GenerateFruit();
	}
}
