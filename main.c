#include <avr/interrupt.h>


volatile unsigned char cont_botoes;
const unsigned char tabela[] = {0XC0, 0XF9, 0XA4, 0XB0, 0X99, 0X92, 0x82, 0XF8, 0x80, 0x90, 0xFF, 0xBF};
unsigned char vect_display[4] = {10, 10, 10, 10};
unsigned char p_display = 0;
unsigned char pwm = 0;
signed char sentido = 1;

void inicio(void)
{
	DDRC = 0xFF;
	PORTC = 0xFF;
	DDRA = 0b11000000;
	PORTA = 0b00111111;
	DDRB = 0b11100000;
	PORTB = 0b00100000;
	OCR0 = 77;
	TCCR0 = 0b00001111;
	TIMSK = 0b00000010;
	OCR2 = 0;
	TCCR2 = 0B01100011;
	sei();
}

void display(void)
{
	PORTA = p_display << 6;
	PORTC = tabela[vect_display[p_display]];
	p_display++;
	if(p_display == 4)
	p_display = 0;
}

ISR(TIMER0_COMP_vect)
{
	display();
	cont_botoes--;
}

void processar_display(void)
{
	vect_display[0] = 10;
	
	if(sentido == -1)
	vect_display[1] = 11;
	else
	vect_display[1] = 10;
	
	if(pwm == 100)
	{
		vect_display[2] = 9;
		vect_display[3] = 9;
	}
	else
	{
		vect_display[2] = pwm / 10;
		vect_display[3] = pwm % 10;
	}
}

void processar_sw(void)
{
	if((PINA & 0b00000001) == 0)
	{
		cont_botoes=1;
		while(cont_botoes!=0);
		if((PINA & 0b00000001) == 0)
		{
			if(pwm+5<=100)
				pwm = pwm + 5;		
		while((PINA & 0b00000001) == 0);
		}
	}
	
	if((PINA & 0b00000010) == 0)
	{	
		if(pwm-5>=0)
			pwm = pwm - 5;
		while((PINA & 0b00000010) == 0);
	}
	
	if((PINA & 0b00000100) == 0)
	{
		pwm = 25;
	}
	
	if((PINA & 0b00001000) == 0)
	{
		pwm = 50;
	}
	
	if((PINA & 0b00010000) == 0)
	{
		cont_botoes=1;
		while(cont_botoes!=0);
		if((PINA & 0b00010000) == 0)
		{
			sentido=-sentido;
			OCR2=0;
			cont_botoes=50;
			while(cont_botoes!=0);
			if(sentido == 1)
			PORTB = (PORTB & 0b10011111) | 0b00100000;
			else
			PORTB = (PORTB & 0b10011111) | 0b01000000;
		}
		
		
		while((PINA & 0b00010000) == 0);	
	}
	
	if((PINA & 0b00100000) == 0)
	{
		pwm = 0;
	}
	
	OCR2 = (pwm * 255) / 100;
}

int main(void)
{
	inicio();
	while(1)
	{
		processar_sw();
		processar_display();
	}
}
