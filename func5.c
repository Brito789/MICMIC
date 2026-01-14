#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdlib.h>

const unsigned char digitos[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xBF, 0xFF};	//{0, 1, 2, 3, 4, 5 , 6 , 7 , 8 , 9 , - , off}
volatile unsigned char valor_display = 0;
volatile unsigned char display_atual = 0;	//0-3 para displays


volatile unsigned contador_tempo = 0;
volatile unsigned char DC = 0;
volatile unsigned char botao = 0;
volatile unsigned char  flag_250ms = 0;
volatile unsigned char estado_inversao = 0; //1-> para o motor 2->espera 250ms
volatile unsigned char flag5ms = 0;
volatile unsigned char velocidade = 0;
volatile unsigned char velocidade_anterior = 0;
volatile unsigned char sentido = 0;		//0 = horário , 1= anti-horário

volatile unsigned char modo_controlo = 'D'; // 'S' para Switches (default) , 'D' para PC, 'A' para o Potênciometro, 'M' para Motor de passo

volatile unsigned char adc_valor_motor = 0; //valor de 0-255 obtido pelo ADC
volatile unsigned char somar_ADC = 0;
volatile unsigned char cnt_ADC = 0;

volatile long posicao_absoluta =0; //posição atual do motor
volatile long posicao_referencia = 0; //posição de referência ('R')
volatile long passos_objetivo = 0;
volatile unsigned char indice_sequencia_mpp = 0; //0,1,2,3 (para full step)
char buffer_passos[10];
volatile int buffer_index = 0;
volatile int sinal_acumulado = 1; //sinal para comandos númericos -1 ou 1
const unsigned char sequencia_full_step[] = {0b00001001,0b00001100,0b00000110,0b00000011};
	
volatile unsigned int t1_stamp = 0, t2_stamp = 0;
volatile unsigned int cont_ovr = 0, t1_ovr = 0;
volatile unsigned char flag_rpm_pronto = 0;
volatile unsigned long rpm = 0;
volatile unsigned long somar_rpm = 0;

//extern unsigned char leitura_adc(void);

void inic(void);
void verificar_switches(void);
void ativar_funcoes(void);
void selecionar_display(unsigned char display_num);
void atualizar_display(void);
void controlar_motor(void);
void inverter_sentido(void);
void parar_motor(void);
void esperar_250ms(void);
void usart1_transmit(unsigned char data);
void usart1_transmit_string(char *str);
void enviar_estado_motor(void);
int ler_ADC(void);
void dar_passo_mpp(int sentido_passo); //sentido_passo é 1 ou -1


int main(void){
	inic();
	sei();
	
	static unsigned char cnt_delay_mpp = 0; //contar os 25ms do motor (5 * 5ms)
	
	while (1)
	{
		if(modo_controlo == 'S'){
			verificar_switches();
			ativar_funcoes();
		}
		else if(modo_controlo == 'D'){
			ativar_funcoes();
		}
		else if(modo_controlo == 'A'){
			//chamar a rotina em assembly
			//rotina assembly calcula a média de 4 leituras
			adc_valor_motor = ler_ADC();
			velocidade  = (adc_valor_motor * 100)/255;
			/*if(cnt_ADC < 4){
				somar_ADC += adc_valor_motor;
				cnt_ADC ++;
			}
			else if(cnt_ADC == 4){
				adc_valor_motor= somar_ADC/4;
				//atualizar a velocidade
				//mapeamento direto: 0-255(ADC)->0-100% velocidade
				velocidade  = (adc_valor_motor * 100)/255;
				cnt_ADC = 0;
			}*/
			verificar_switches();
			ativar_funcoes();
		}
		
		
		else if (modo_controlo == 'V'){
			verificar_switches();
			ativar_funcoes();
		}
		
		if(flag5ms == 1){
			flag5ms = 0;
			
			if(modo_controlo == 'M'){
				cnt_delay_mpp ++;
				
				if(cnt_delay_mpp >= 5){
					cnt_delay_mpp = 0;
					
					if(posicao_absoluta < passos_objetivo){
						dar_passo_mpp(1);
					}
					else if (posicao_absoluta > passos_objetivo){
						dar_passo_mpp(-1);
					}
				}
			}
			atualizar_display();
		}
		
		if (flag_rpm_pronto) {
			flag_rpm_pronto = 0;
			if (t1_stamp == 0) {
				rpm = 0;
				} else {
				// Delta de ticks considerando os overflows
				long delta_ticks = ((long)t1_ovr * 65536) + t1_stamp - t2_stamp;
				//long delta_ticks = t2_stamp-t1_stamp;
				if (delta_ticks > 0) {
					// RPM = 60 * (f_timer / delta_ticks) / impulsos_por_volta
					// Para Prescaler 64: f_timer = 16MHz / 64 = 250.000 Hz
					// Ttick = 1/250.000 = 0,000004
					//Tpulse = delta_tick * Ttick
					//fpulse = 1/Tpulse = 1/(delta_tick * 0,000004)
					//RPM = 60*((fpulse)/2) = 60* ((1/(delta_tick * 0,000004))/2)
					// RPM = (60 * 250000) / (delta_ticks * 2) = 7.500.000 / delta_ticks
					//rpm = 60/ ((250000*(delta_ticks))/2);
					rpm = 7500000/ (delta_ticks);
					
					
				}
			}
		}
		
		inverter_sentido();
		controlar_motor();

		
	}
}

ISR(TIMER0_COMP_vect){
	flag5ms = 1;
	contador_tempo++;
	if(contador_tempo == 50){		// 50 * 5ms = 250ms
		flag_250ms = 1;
	}
}

ISR(USART1_RX_vect){
	unsigned char caracter_recebido = UDR1;
	usart1_transmit(caracter_recebido);
	
	if(caracter_recebido == 's' || caracter_recebido == 'S'){
		modo_controlo = 'S';
		buffer_index = 0;
		sinal_acumulado = 1;
		velocidade = 0;
	}
	else if(caracter_recebido == 'd' || caracter_recebido == 'D'){
		modo_controlo = 'D';
		buffer_index = 0;
		sinal_acumulado = 1;
		velocidade = 0;
	}
	else if(caracter_recebido == 'a' || caracter_recebido == 'A'){
		modo_controlo = 'A';
		buffer_index = 0;
		sinal_acumulado = 1;
		velocidade = 0;
	}
	else if( caracter_recebido == 'm' || caracter_recebido == 'M'){
		modo_controlo = 'M';
		buffer_index = 0;
		sinal_acumulado = 1;
		velocidade = 0;
	}
	else if(caracter_recebido == 'v' || caracter_recebido == 'V'){
		modo_controlo = 'V';
		buffer_index = 0;
		sinal_acumulado = 1;
		velocidade = 0;
	}
	
	if(modo_controlo == 'D'){
		switch(caracter_recebido){
			case 'p':  //parar
			velocidade = 0;
			break;
			
			case '+': //incrementar 5%
			velocidade += 5;
			if(velocidade> 100){
				velocidade = 100;
			}
			break;
			
			case '-': //decrementar 5%
			if(velocidade >=5){
				velocidade -= 5;
				}else{
				velocidade = 0;
			}
			break;
			
			case '1': //25%
			velocidade = 25;
			break;
			
			case '2': //50%
			velocidade = 50;
			break;
			
			case 'i': //inverter sentido
			botao = 5;
			velocidade_anterior = velocidade;
			parar_motor();
			estado_inversao = 1;
			break;
			
			case 'b': //mostrar velocidade do motor e sentido de rotação
			botao = 7;
			break;
			
			default:
			break;
		}
	}
	else if(modo_controlo == 'M'){
		if (caracter_recebido == '-'){
			sinal_acumulado = -1;
		}
		
		else if(caracter_recebido >= '0' && caracter_recebido <= '9'){
			//acumular o dígito no buffer
			if (buffer_index <9){
				buffer_passos[buffer_index++] = caracter_recebido;
			}
		}
		
		else if(caracter_recebido == '\r' || caracter_recebido == '\n'){
			//enter: processar os passos do buffer e iniciar a movimentação
			if (buffer_index > 0){
				buffer_passos[buffer_index] = '\0';
				
				int valor_digitado = atoi(buffer_passos);
				
				passos_objetivo = posicao_referencia + (valor_digitado * sinal_acumulado);
				buffer_index = 0;
				sinal_acumulado = 1;
			}
		}
		else if(caracter_recebido == 'Z' || caracter_recebido == 'z'){ //incrementa 1 passo
			dar_passo_mpp(1);
			passos_objetivo = posicao_absoluta;
			
			
		}
		else if(caracter_recebido == 'X' || caracter_recebido == 'x'){ //decrementa 1 passo
			dar_passo_mpp(-1);
			passos_objetivo = posicao_absoluta;
			
			
		}
		else if(caracter_recebido == 'R' || caracter_recebido == 'r'){
			posicao_referencia = posicao_absoluta;
			passos_objetivo = posicao_absoluta;
		}
	}
	
	
	
}

ISR(TIMER1_CAPT_vect){
	t2_stamp = t1_stamp;
	t1_stamp = ICR1;       // Guarda o valor capturado do Timer1
	t1_ovr = cont_ovr;     // Guarda quantos overflows ocorreram entre impulsos
	cont_ovr = 0;          // Faz reset ao contador de overflows
	flag_rpm_pronto = 1;   // Sinaliza ao main que pode calcular
}

ISR(TIMER1_OVF_vect){
	cont_ovr++;
	if (cont_ovr > 50) { // Valor MAX para considerar o motor parado
		t1_stamp = 0;
		t1_ovr = 0;
		cont_ovr = 0;
		flag_rpm_pronto = 1;
	}
}
void inic(void){
	//PORTC -> Display
	DDRC = 0xFF;
	PORTC = 0xFF;
	
	//PORTA -> Switches
	DDRA = 0b11000000;
	PORTA = 0b00111111;
	
	//PORTB -> Motor
	DDRB = 0b11100000;
	PORTB &= ~((1<<PB6)|(1<<PB5));
	
	//TIMER0 -> 5ms modo CTC
	OCR0 = 77;
	TCCR0 = (1<<WGM01)|(1<<CS02)|(1<<CS01)|(1<<CS00);
	TIMSK = (1<<OCIE0);
	
	//TIMER2 -> modo PWM
	TCCR2 = (1<< WGM20)|(1<<COM21)|(1<<CS21)|(1<<CS20); //Configurar PWM prescaler 64 fpwm= 500Hz
	OCR2 = 0;
	
	//USART1 -> UBRR1L e UBRR1H UCSR1A UCSR1B UCSR1C UDR1
	UBRR1H = 0;
	UBRR1L = 207;
	UCSR1A = (1<<U2X1);
	UCSR1B = (1<<RXCIE1) | (1<<RXEN1) | (1<<TXEN1);
	UCSR1C =  (1<<USBS1) | (1<<UCSZ11) | (1<<UCSZ10);
	
	//ADC -> canal 1
	ADMUX = 0b00100001;
	ADCSRA = 0b10000111;
	
	//PORTE -> Motor de Passo-a-Passo (PE0-PE3)
	DDRE |= 0b00001111; //PE0 a PE3 são saídas
	PORTE = 0b00000000;
	
	//TIMER1 ->modo normal
	TCCR1A = 0;
	TCCR1B = (1 << ICNC1) | (1 << ICES1) | (1 << CS11) | (1 << CS10);
	TIMSK |= (1 << TICIE1) | (1 << TOIE1);
	
	//PORTD ->PD4 -ENTRADA
	DDRD = 0b00000000;
}

void verificar_switches(void){
	static unsigned char estado_anterior = 0b00111111;
	unsigned char estado_atual = PINA & 0b00111111;
	if(estado_atual != estado_anterior){
		if(estado_anterior == 0b00111111){
			switch(estado_atual){
				case 0b00111110:	//SW1 ATIVO
				botao = 1;
				break;
				
				case 0b00111101:	//SW2 ATIVO
				botao = 2;
				break;
				
				case 0b00111011:	//SW3 ATIVO
				botao = 3;
				break;
				
				case 0b00110111:	//SW4 ATIVO
				botao = 4;
				break;
				
				case 0b00101111:	//SW5 ATIVO
				botao = 5;
				velocidade_anterior = velocidade;
				parar_motor();
				estado_inversao = 1;
				break;
				
				case 0b00011111:	//SW6 ATIVO
				botao = 6;
				break;
				
				default:
				botao = 0;
				break;
			}
			
		}
		estado_anterior = estado_atual;
	}
}

void ativar_funcoes(void){
	if (modo_controlo == 'S' ||  modo_controlo == 'V'){
		if(botao != 0){
			switch(botao){
				case 1:				// SW1 -> Incrementa 5%
				if(velocidade <100){
					velocidade += 5;
					if(velocidade >100){
						velocidade = 100;
					}
					
				}
				break;
				
				case 2:				//SW2 ->Decrementa 5%
				if(velocidade>=5){
					velocidade-=5;
					}else{
					velocidade = 0;
				}
				
				break;
				
				case 3:				//SW3 -> 25% velocidade nominal
				velocidade = 25;
				break;
				
				case 4:				//SW4 -> 50% velocidade nominal
				velocidade = 50;
				break;
				
				case 5:				//SW5 -> inverter sentido
				break;
				
				case 6:				//SW6 -> parar motor
				parar_motor();
				break;
				
				case 7:
				enviar_estado_motor();
				break;
				
				default:
				botao =0;
			}
			botao = 0;

		}
	}
	else if (modo_controlo == 'A')
	{
		//sw5 (inversão de sentido)
		if (botao != 0)
		{
			switch(botao){
				case 5:
				botao = 5;
				velocidade_anterior = velocidade;
				parar_motor();
				estado_inversao = 1;
				break;
				
				default:
				botao = 0;
			}
			botao = 0;
		}
	}
}

void selecionar_display(unsigned char display_num){
	// display_num: 0->unidades 1->dezenas 2->sinal
	unsigned char temp = PORTA & 0b00111111;
	unsigned char select_display = 0;
	switch(display_num){
		case 0:		//Display 0 (mais à direita) -> PA7=1, PA6=1
		select_display = 0b11000000;
		break;
		case 1:		//Display 1 -> PA7=1, PA6=0
		select_display = 0b10000000;
		break;
		case 2:		//Display 2 -> PA7=0, PA6=1
		select_display = 0b01000000;
		break;
		case 3:		//Display 3 -> PA7=1, PA6=1
		select_display = 0b00000000;
		break;
		
	}
	PORTA = select_display | temp;
}

void atualizar_display(void){
	static unsigned char contador_multiplex = 0;
	
	unsigned char velocidade_display = 0;
	if (velocidade > 99)
	{
		velocidade_display = 99;
	}
	else{
		velocidade_display = velocidade;
	}
	
	long passos_pendentes = posicao_absoluta - posicao_referencia;
	long passos_absolutos = 0;
	
	if(passos_pendentes < 0){
		passos_absolutos = -passos_pendentes;
	}
	else{
		passos_absolutos = passos_pendentes;
	}
	
	
	
	if (modo_controlo == 'M')
	{
		switch(contador_multiplex){
			case 0:
				valor_display = digitos[passos_absolutos % 10];
				selecionar_display(0);
				break;
			case 1:
				if(passos_pendentes >=10){
					valor_display = digitos[(passos_absolutos/10) % 10];
				}
				else{
					if(passos_pendentes >0){
						valor_display = digitos[10];
					}
					else{
						valor_display = digitos[11];
					}
				}
				selecionar_display(1);
				break;
			case 2:
				if(passos_pendentes >= 100){
					valor_display = digitos[(passos_absolutos/100)%10];
				}
				else if (passos_pendentes > 0 && passos_absolutos >= 10){
					valor_display = digitos[10];
				}
				else{
					valor_display = digitos[11];
				}
				selecionar_display(2);
				break;
			case 3:
				if(passos_pendentes<99){
					valor_display = digitos[11];
				}
				else{
					valor_display = digitos[10];
				}
			
				selecionar_display(3);
				break;
		}
		PORTC = valor_display;
		contador_multiplex = (contador_multiplex + 1) % 4;
	}
	else if(modo_controlo == 'V'){
		switch(contador_multiplex){
			case 0:
				valor_display = digitos[rpm % 10];
				selecionar_display(0);
				break;
			case 1:
				valor_display = digitos[(rpm/10)%10];
				selecionar_display(1);
				break;
			case 2:
				valor_display = digitos[(rpm / 100)%10];
				selecionar_display(2);
				break;
			case 3:
				valor_display = digitos[(rpm /1000)%10];
				selecionar_display(3);
				break;
		}
		PORTC = valor_display;
		contador_multiplex = (contador_multiplex + 1) % 4;
	}
	
	else {
		
		
		if(contador_multiplex == 0){
			valor_display = digitos[velocidade_display % 10];
			selecionar_display(0);
			contador_multiplex = 1;
			
		}
		else if(contador_multiplex == 1){
			if(velocidade_display <10){
				if(sentido == 0){
					valor_display = digitos[11];
				}
				else{
					valor_display = digitos[10];
				}
				}else{
				valor_display = digitos[(velocidade_display/10)%10];
			}
			selecionar_display(1);
			contador_multiplex=2;
			
		}
		else if(contador_multiplex == 2){
			if(velocidade_display >= 10){
				if(sentido == 0){
					valor_display = digitos[11];
				}
				else{
					valor_display = digitos[10];
				}
			}
			else{
				valor_display = digitos[11];
			}
			selecionar_display(2);
			contador_multiplex=3;
			
		}
		else if(contador_multiplex == 3){
			if( modo_controlo == 'D' ){
				valor_display = 0b10100001;
			
			}
			else if(modo_controlo == 'S'){
				valor_display = digitos[5];
			}
			else if(modo_controlo == 'A'){
				valor_display = 0b10001000;
			}
			selecionar_display(3);
			contador_multiplex=0;
			
		}
		PORTC = valor_display;
	}
	
	
}

void controlar_motor(void){
	DC = (velocidade * 255)/100;
	
	OCR2 = DC;
	
	if(velocidade == 0){
		//Motor Parado
		PORTB &= ~((1<<PB5)|(1<<PB6));
		
		}else{
		if(sentido == 0){
			//Sentido Horário PB5 =0  PB6 =1
			PORTB |= (1<<PB6);
			PORTB &= ~(1<<PB5);
		}
		else{
			//Sentido Anti-Horário PB5=1 PB6=0
			PORTB &= ~(1<<PB6);
			PORTB|= (1<<PB5);
		}
	}
}

void inverter_sentido(void){
	if(estado_inversao == 1){
		if(flag_250ms == 1){
			//limpar as flags de delay
			flag_250ms = 0;
			contador_tempo = 0;
			
			//inverter o sentido
			if(sentido == 0){
				sentido=1;
				}else{
				sentido=0;
			}
			//restuara a velocidade inicial
			velocidade = velocidade_anterior;
			estado_inversao = 0;
		}
	}
}

void parar_motor(void){
	velocidade = 0;
	controlar_motor();
}

void usart1_transmit(unsigned char data){
	//esperar que o buffer de transmissão esteja vazio (UDRE1 = 1)
	while((UCSR1A & (1<<UDRE1))==0);
	//coloca o dado no buffer e envia o dado
	UDR1= data;
}

void usart1_transmit_string(char *str){
	for(size_t i = 0; i< strlen(str);i++){
		usart1_transmit(str[i]);
	}
}

void enviar_estado_motor(void){
	char buffer[50];
	const char *str_sentido;
	
	if(sentido == 0){
		str_sentido = "HORARIO";
		}else{
		str_sentido = "ANTI-HORARIO";
	}
	//Mensagem no buffer exemplo : "Velocidade: 50% | Sentido: HORARIO\r\n"
	sprintf(buffer, "Velocidade: %d%% | Sentido: %s\r\n",velocidade,str_sentido);
	
	usart1_transmit_string(buffer);
}

int ler_ADC(void){
	unsigned char leituraH;
	
	ADCSRA |= (1<<ADSC);
	
	while ((ADCSRA & (1<<ADSC)) != 0);
	
	leituraH = ADCH;
	
	return (leituraH);
}

void dar_passo_mpp(int sentido_passo){
	//atualiza a posição absoluta
	posicao_absoluta +=sentido_passo;
	
	if(sentido_passo > 0){ //incremente sentido horário
		indice_sequencia_mpp = (indice_sequencia_mpp + 1) % 4; //resto da divisão cria um ciclo
		}else{
		if (indice_sequencia_mpp == 0){
			indice_sequencia_mpp = 3;
			}else{
			indice_sequencia_mpp --;
		}
	}
	
	PORTE = sequencia_full_step[indice_sequencia_mpp];
}