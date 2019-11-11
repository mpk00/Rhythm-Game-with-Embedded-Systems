/*
 * GccApplication13.c
 *
 * Created: 3/5/2019 1:13:26 PM
 * Author : Moses
 */ 

#include <avr/io.h>
#include "clock.h"

//=======================   FOR SHIFT REGISTER   =================================
unsigned char setBit(unsigned char number, unsigned char position, unsigned short val)
{ 	if(val == 1) {unsigned char num = 0x01 << position; return number | num;}
if(val == 0) {unsigned char num = 0x01 << position; return number & ~num;}
}

unsigned char getBit(unsigned char number, unsigned char position)
{
	return (number >> position) & 0x01;
}

void shift(unsigned char number)
{
	// toggle srclock after pushing each bit of the number
	for(int i = 7; i >= 0; i--)
	//for(int i = 0; i < 8; i++)		<--- this is for reverse order of pushing bits
	{
		PORTC = setBit(PORTC, 0, getBit(number, i));
		PORTC = setBit(PORTC, 2, 0);
		PORTC = setBit(PORTC, 2, 1);
	}
	
	// set rclock from 0 to 1 to output results to shift register
	PORTC = setBit(PORTC, 1, 0);
	PORTC = setBit(PORTC, 1, 1);

	

}
// ==============================================================================================================
//		ADC FOR JOYSTICK

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);

}

void Set_a2d_Pin(unsigned char pinNum)
{
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	static unsigned char i = 0;
	for(i = 0; i < 15; i++)
	{
		asm("nop");
	}
}

// ===============================================================================================================

//================================================================================
//   STATE MACHINE 1

enum state {start, init, gameplay, scoreDisplay} state;
unsigned char score = 0;
void tick(){
	unsigned short joystick = ADC;
	unsigned char posX, posY;
	unsigned char terminate = 0x00;
	
	switch(state){//transitions
		case start:
			state = init;
			break;
		case init:
			if(joystick > 700 || joystick < 300) {state = gameplay;}
			else {state = init;}
			break;
		case gameplay:
			if(~PINA & 0x10) {state = init;}
			else if(posX == 128 && posY == 0x1F && joystick > 300) {state = scoreDisplay;}
			else {state = gameplay;}
		case scoreDisplay:
			if(~PINA & 0x10) {state = init;}
			else {state = scoreDisplay;}

	}
	
	switch(state){//actions
		case init:
			PORTD = 0xE7;
			shift(0x18);
			score = 0;
			break;
		case gameplay:
			posX = 0x01;
			posY = 0x1F;
			
			TimerSet(100);
			TimerOn();

			while(terminate == 0x00)
			{
				joystick = ADC;
				if(posX < 128) { posX = posX * 2; }
				
				
				// when player scores lower goal
				if(posX >= 64 && posY == 0x1F)
				{
					if(joystick < 300) {score++; posX = 0x01; posY = 0xF8;}
				}
			
				// when player scores upper goal
				if(posX >= 64 && posY == 0xF8)
				{
					if(joystick > 700) {score++; posX = 0x01; posY = 0x1F;}
				}
				
				
				// player loses handler
				if(posX == 128)
				{
					// for either case, exit gameplay and into scoreDisplay
					if(posY == 0x1F && joystick > 300) {terminate = 0x01;}
					if(posY == 0xF8 && joystick < 700) {terminate = 0x01;}
				} 
				
				//--------------------------------------
				PORTD = posX;
				shift(posY);

				while (!TimerFlag) {} // Wait for timer period
				TimerFlag = 0;
			}
			
			
			break;
			
		case scoreDisplay:			
			PORTD = 0x01;
			shift(score);
			break;
	}
}


int main(void)
{
	 DDRD = 0xFF; PORTD = 0x00;
	 DDRC = 0xFF; PORTC = 0x00;
	 DDRA = 0x00; PORTA = 0xFF;
    /* Replace with your application code */
	

	ADC_init();
	Set_a2d_Pin(0x01);
	while(1)
	{
		tick();
	} 
	
	

}

