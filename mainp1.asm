;
; Projeto 1 enois.asm
;
; Created: 03/10/2025 11:24:43
; Author : DEE
;


.include <m128def.inc>
.cseg
.EQU Sw1=0
.EQU Sw2=1
.EQU Sw3=2
.EQU Sw4=3
.EQU Sw6=5
.EQU LED1=0
.EQU LED2=1
.EQU LED3=2
.EQU LED4=3
.EQU LED5=4
.EQU LED6=5
.EQU LED7=6
.EQU LED8=7
.org 0
	
	CLR		R16
	OUT		DDRA,R16
	LDI		R16,0B11111111
	OUT		DDRC,R16
	OUT		PORTC,R16

LER:
	SBIS PINA, SW1
	JMP SW1ON
	SBIS PINA,SW2
	JMP SW2ON
	SBIS PINA, SW3
	JMP SW3ON
	SBIS PINA,SW4
	JMP SW4ON
	SBIS PINA,SW6
	JMP SW6ON
	JMP LER
SW1ON:
	CBI PORTC,LED1
	CBI PORTC,LED8
	JMP LER
SW2ON:
	CBI PORTC,LED2
	CBI PORTC,LED7
	JMP LER
SW3ON:
	CBI PORTC,LED3
	CBI PORTC,LED6
	JMP LER
SW4ON:
	CBI PORTC,LED4
	CBI PORTC,LED5
	JMP LER
SW6ON:
	LDI		R16,0B11111111
	OUT		PORTC,R16
	JMP		LER




delay_1s:	push r16
			push r17
			push r18

			ldi r18,82
ciclo0:		ldi r17,255
ciclo1:		ldi r16,255
ciclo2:		dec r16
			brne ciclo2
			dec r17
			brne ciclo1
			dec r18
			brne ciclo0

			pop r18
			pop r17
			pop r16
			ret 

delay_500ms:	
			push r16
			push r17
			push r18

			ldi r18,41
ciclo3:		ldi r17,255
ciclo4:		ldi r16,255
ciclo5:		dec r16
			brne ciclo5
			dec r17
			brne ciclo4
			dec r18
			brne ciclo3

			pop r18
			pop r17
			pop r16
			ret