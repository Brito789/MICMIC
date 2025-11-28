/*
 * enoisC2.c
 *
 * Created: 28/11/2025 12:18:38
 * Author : DEE
 */ 

#include <avr/interrupt.h>


unsigned char cont, cont_500;
const unsigned char tabela[]= {0XC0, 0Xf9, 0XA4, 0XB0, 0X99, 0X92, 0x82, 0xF8, 0x80, 0x90, 0xFF, 0xbf};
unsigned char vect_display[4]={10,10,10,10};
unsigned char p_display=0;
volatile unsigned int pwm=0;

void inicio(void)
{
	DDRC= 0XFF;
	PORTC= 0XFF;
	DDRA= 0b11000000;
	PORTA= 0b11000000;
	DDRB=0b11100000;
	PORTB=0b00100000;
	OCR0= 77;
	TCCR0= 0b00001111;
	TIMSK= 0b00000010;
	OCR2=0;
	TCCR2=0B01100011;
	sei();
	
}

void display(void)
{
	PORTA=p_display << 6;
	PORTC=tabela[vect_display[p_display]];
	p_display++;
	if(p_display==4)
		p_display=0;
}

ISR(TIMER0_COMP_vect)
{
	display();
}

processar_display()
{
	if(pwm=100)
	{
		vect_display[0]=10;
		vect_display[1]=10;
		vect_display[2]=9;
		vect_display[3]=9;
	}else
	{
		vect_display[0]=10;
		vect_display[1]=10;
		vect_display[2]=pwm/10;
		vect_display[3]=pwm%10;
	}
}

processar_sw()
{
	if(PINA & 0b00000001==0)
	{
		pwm=+5;
	}
	if(PINA & 0b00000010==0)
	{
		pwm=-5;
	
	if(PINA & 0b00000100==0)
	{
		pwm=25;
	}
	if(PINA & 0b00001000==0)
	{
		pwm=50;
	}
	if(PINA & 0b00010000==0)
	{
		pwm=-pwm;
		if(vect_display[1]=10){
			vect_display[1]=11;
		}else{
			vect_display[1]=10;
		}
	}
	if(PINA & 0b00100000==0)
	{
		pwm=0;
	}
}
	
	
}
int main(void)
{
    inicio();
    while (1) 
    {
		processar_sw();
		processar_display();
    }
}

