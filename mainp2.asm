;
; ENOIS2.asm
;
; Created: 10/10/2025 12:05:53
; Author : DEE
;
;
; Projeto 1 enois.asm
;
; Created: 03/10/2025 11:24:43
; Author : DEE
;


.include <m128def.inc>
.cseg
.def LEDX=R23
.EQU Sw1=0
.EQU Sw2=1
.EQU Sw6=5
.EQU LED1=0
.EQU LED8=7
.org 0
jmp main
	
inicio:
	CLR		R16
	OUT		DDRA,R16
	LDI		R16,0B11111111
	OUT		DDRC,R16
	OUT		PORTC,R16
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

main:
	ldi r16,0xff
	out spl,r16
	ldi r16,0x10
	out sph,r16
	call inicio
	jmp ler


LER:
	SBIS PINA, SW1
	JMP SW1ON
	SBIS PINA,SW2
	JMP SW2ON
	JMP LER

SW1ON:
	LDI		LEDX,0B11111111 ;desligar 
	OUT		PORTC,LEDX	   ; LEDS
	call	delay_1s
CicloSW1:
	lsl		LEDX
	out		PORTC, LEDX
	call	delay_1s
	SBIS PINA,SW6
CICLO22:
	SBIS PINA, SW1
	JMP BORA
	JMP CICLO22

BORA:
	SBIS PORTC, 7
	JMP LER
	JMP CicloSW1

	

SW2ON:
	LDI		LEDX,0B00000000
	OUT		PORTC,LEDX	   ; LEDS
	call	delay_500ms
CICLOSW2:
	SEC
	ROR		LEDX
	out		PORTC, LEDX
	call	delay_500ms
	SBIS	PINA,SW6

CICLO33:
	SBIS PINA,SW2
	JMP BORA1
	JMP CICLO33

BORA1:
	SBIC PORTC, 0
	JMP LER
	JMP CICLOSW2
	

	




