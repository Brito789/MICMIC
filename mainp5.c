
#include <avr/interrupt.h>

unsigned char cont, cont_500;

typedef struct
{
      unsigned char STOP:1;
      unsigned char FLAGTIMER:1;
}
FLAGS_ST;

volatile FLAGS_ST flags = { 0, 0};

const unsigned char tabela[]= {0XC0, 0Xf9, 0XA4, 0XB0, 0X99, 0X92};

void inicio(void)
{
      DDRC= 0XFF;
PORTC= 0XFF;
DDRD= 0b11000000;
PORTD= 0b11000000;
OCR0= 249;
TCCR0= 0b00001101;
TIMSK= 0b00000010;
EICRA= 0b00001000;
      sei();
      SREG|= 0X80;
}

ISR(INT1_vect)
{
      flags.STOP=1;
}

ISR(TIMER0_COMP_vect)
{
      cont_500--;
      if(cont_500 == 0)
      {
            cont_500=250;
            flags.FLAGTIMER=1;
      }
}

void display(void)
{
      PORTC = tabela[cont];
}

void piscar(void)
{
      unsigned char cont_aux=3;
      while(cont_aux!=0)
      {
            cont_500=250;
            flags.FLAGTIMER=0;
            while(flags.FLAGTIMER==0);
            PORTC= 0XFF;
            cont_500=250;
            flags.FLAGTIMER=0;
            while(flags.FLAGTIMER==0);
            display();
            cont_aux--;
      }
      flags.STOP=0;
      flags.FLAGTIMER=0;
      EIMSK=0X00;
}

int main(void)
{
      inicio();
      while(1)
      {
            while((PIND &  0b00000001)!= 0);
            EIMSK=0b00000010;
            cont_500=250;
            cont=0;
            flags.FLAGTIMER=0;
            display();
            while(flags.STOP==0)
            {
                  if(flags.FLAGTIMER==1)
                  {
                        flags.FLAGTIMER=0;
                        cont++;
                        if(cont==6)
                        {
                              cont=0;
                        }
                        display();
                  }
            }
            piscar();
      }
}

