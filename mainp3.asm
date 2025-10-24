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
	LDI		CONT, 9
	ret

delay_1s:
	push r16
	push r17
	push r18
	ldi r18,82
ciclo0:
	ldi r17,255
ciclo1:
	ldi r16,255
ciclo2:
	dec r16
	brne ciclo2
	dec r17
	brne ciclo1
	dec r18
	brne ciclo0
	pop r18
	pop r17
	pop r16
	ret
			
delay_2ms:
	push r16
	push r17
	push r18
	ldi r18,16
ciclo3:
	ldi r17,100
ciclo4:
	ldi r16,100
ciclo5:
	dec r16
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

LER:
	SBIS PIND, S
	JMP S_ON
	SBIS PIND, NI
	JMP NI_ON
	SBIS PIND, ND
	JMP ND_ON
	JMP LER

S_ON:
	call delay_2ms
	SBIC PIND, S
	jmp LER
MOTOR:
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
	CALL Display
	CPI CONT, 0
	BREQ cont_zero
	JMP MOTOR

cont_zero:
	CALL Display
	JMP LER

ND_ON:
	call delay_2ms
	SBIC PIND, ND
	jmp LER
	DEC CONT
	CPI CONT, 0
	BRNE mostra_display
	LDI CONT, 1
mostra_display:
	CALL Display
esperar_nd:
	SBIC PIND, ND
	jmp LER
	jmp esperar_nd


NI_ON:
	call delay_2ms
	SBIC PIND, NI
	jmp LER
	INC CONT
	CPI CONT, 10
	BRNE mostra_display_ni
	LDI CONT, 9
mostra_display_ni:
	CALL Display
esperar_ni:
	SBIC PIND, NI
	jmp LER
	jmp esperar_ni

Display:
    CPI     CONT, 0
    BREQ    show_1
    CPI     CONT, 1
    BREQ    show_1
    CPI     CONT, 2
    BREQ    show_2
    CPI     CONT, 3
    BREQ    show_3
    CPI     CONT, 4
    BREQ    show_4
    CPI     CONT, 5
    BREQ    show_5
    CPI     CONT, 6
    BREQ    show_6
    CPI     CONT, 7
    BREQ    show_7
    CPI     CONT, 8
    BREQ    show_8
    CPI     CONT, 9
    BREQ    show_9
    JMP     show_9


show_1: 
    LDI R16, 0xF9
    OUT PORTC, R16
    RET

show_2: 
    LDI R16, 0xA4
    OUT PORTC, R16
    RET

show_3: 
    LDI R16, 0xB0
    OUT PORTC, R16
    RET

show_4: 
    LDI R16, 0x99
    OUT PORTC, R16
    RET

show_5: 
    LDI R16, 0x92
    OUT PORTC, R16
    RET

show_6: 
    LDI R16, 0x82
    OUT PORTC, R16
    RET

show_7: 
    LDI R16, 0xF8
    OUT PORTC, R16
    RET

show_8: 
    LDI R16, 0x80
    OUT PORTC, R16
    RET

show_9: 
    LDI R16, 0x90
    OUT PORTC, R16
    RET
