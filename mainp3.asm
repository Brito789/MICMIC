;
; enois3.asm
;
; Created: 17/10/2025 12:02:45
; Author : DEE
;


.include <m128def.inc>
.cseg
.def CONT=R22
.EQU S=0
.EQU NI=1
.EQU ND=2
.EQU SP=5
.def LEDX=R23
.org 0
jmp main

inicio:
	LDI		R16,0B11111111
	OUT		DDRA,R16
	OUT		DDRC,R16
	OUT		PORTA,R16
	LDI		R16,0X90
	OUT		PORTC,R16
	LDI		R16,0B11000000
	OUT		PORTD,R16
	OUT		DDRD,R16
	ret


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
			
			
delay_2ms:	push r16
			push r17
			push r18

			ldi r18,16
ciclo3:		ldi r17,25
ciclo4:		ldi r16,25
ciclo5:		dec r16
			brne ciclo2
			dec r17
			brne ciclo1
			dec r18
			brne ciclo0

			pop r18
			pop r17
			pop r16
			ret  

main:
	ldi r16,0xff
	out spl,r16
	ldi r16,0x10
	out sph,r16
	call inicio


LER:
	SBIS PIND, S ;S
	JMP S_ON
	SBIS PIND, NI ;NI
	JMP NI_ON
	SBIS PIND, ND ; ND
	JMP ND_ON
	JMP LER

S_ON:
	call delay_2ms
	SBIC PIND, S
	jmp LER
	LDI LEDX,0b01111111
	OUT PORTA, LEDX

SPOON:
	SBIS PIND,SP
	JMP SP_ON
	JMP SPOON

SP_ON:
	LDI LEDX, 0b11111110
	OUT PORTA, LEDX
	call delay_1s
	LDI LEDX, 0b11111111
	OUT PORTA, LEDX
	call delay_1s
	DEC CONT


ND_ON:
	call delay_2ms
	SBIC PIND, ND
	jmp LER

NI_ON:
	call delay_2ms
	SBIC PIND, NI
	jmp LER


Display:
	; para cada valor do cont equivale a um numero do display
	; utilizae DEC e INC para o cont