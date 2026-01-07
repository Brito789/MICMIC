#include <avr/interrupt.h>
#include <stdio.h>

char buffer[50];
volatile unsigned char cont_botoes;
const unsigned char tabela[] = {0XC0, 0XF9, 0XA4, 0XB0, 0X99, 0X92, 0x82, 0XF8, 0x80, 0x90, 0xFF, 0xBF};
unsigned char vect_display[4] = {10, 10, 10, 10};
unsigned char p_display = 0;
unsigned char pwm = 0;
signed char sentido = 1;
volatile unsigned char flag_Rx, caracter;
unsigned char modo_funcionamento = 0; 

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
	UBRR1H=0;
	UBRR1L=103;
	UCSR1A=0b00000000;
	UCSR1B=0b10011000;
	UCSR1C=0b00001110;
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

ISR(USART1_RX_vect)
{
	caracter=UDR1;
	flag_Rx=1;
}

void enviar_Buffer(void)
{
	unsigned char i=0;
	while(buffer[i]!=0)
	{
		while(!(UCSR1A & (1<<UDRE1)));
		UDR1=buffer[i];
		i++;
	}
}


void processar_display(void)
{

	
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

void processar_rx(void)
{
	switch(caracter)
	{
		case'p':
		case'P':
		pwm=0;
		sprintf(buffer,"Motor parado \n\r");
		enviar_Buffer();
		break;
	
		case 'I':
		case 'i':
		sentido=-sentido;
		OCR2=0;
		cont_botoes=50;
		while(cont_botoes!=0);
		if(sentido == 1)
		PORTB = (PORTB & 0b10011111) | 0b00100000;
		else
		PORTB = (PORTB & 0b10011111) | 0b01000000;
		break;
	
		case '1':
		pwm = 25;
		sprintf(buffer,"Velocidade definida para 25 \n\r");
		enviar_Buffer();
		break;
	
		case '2':
		pwm=50;
		sprintf(buffer,"Velocidade definida para 50 \n\r");
		enviar_Buffer();
		break;
	
		case '+':
		if(pwm+5<=100)
		pwm = pwm + 5;
		sprintf(buffer,"Velocidade aumentada para %d%% \n\r", pwm);
		enviar_Buffer();
		break;
	
		case '-':
		if(pwm-5>=0)
		pwm = pwm - 5;
		sprintf(buffer,"Velocidade diminuida para %d%% \n\r", pwm);
		enviar_Buffer();
		break;
		
		case 'b':
		case 'B':
		sprintf(buffer, "Velocidade atual:%d%% \n\r", pwm);
		enviar_Buffer();
		break;
		
	}
	OCR2 = (pwm * 255) / 100;

}
int main(void)
{
	inicio();
	while(1)
	{
		processar_display();
		if(flag_Rx==1 &&(caracter=='d' || caracter=='D'))
		{
			
		modo_funcionamento=0;
		sprintf(buffer, "Modo Digital \n\r");
		enviar_Buffer();
		vect_display[0]=0;
		flag_Rx=0;
		}
		if(flag_Rx==1 &&(caracter=='s' || caracter=='S'))
		{
		modo_funcionamento=1;
		sprintf(buffer, "Modo Switch \n\r");
		enviar_Buffer();
		vect_display[0]=5;
		flag_Rx=0;
		}
		
		if(modo_funcionamento==0 && flag_Rx ==1)
			{
					processar_rx();
					flag_Rx = 0;
			}
		if(modo_funcionamento==1)
		{
			processar_sw();	
		}
			
	}

}
