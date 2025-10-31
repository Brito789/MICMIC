;
; tiroteio.asm
;
; Created: 31/10/2025 12:44:56
; Author : DEE
;


; Replace with your application code
.include <m128def.inc>
.cseg
.equ SW_INICIO = 0
.equ SW_PARAR = 1
.equ PROX_TIMER = 3
.equ FLAG_TIMER = 4
.def FLAGS = r23
.def CONTADOR = r24
.def CONTADOR0 = r17

.org 0
jmp main

.org 0x04 
jmp INT1_ISR

.org 0x1E
jmp TIMER0_ISR

.org 0x46

CONFIGURAR:
    ldi r16, 0b11000000
    out DDRD, r16
    out PORTD, r16
    ser r16
    out DDRA, r16
    out PORTA, r16
    out DDRC, r16
    out PORTC, r16
    ldi r16, 249
    out OCR0, r16
    ldi r16, 0b00001101
    out TCCR0, r16
    ldi r16, 0b00000010
    out TIMSK, r16
    ldi r16, 0b00001000
    sts EICRA, r16
    ldi r16, 0b00000000
    out EIMSK, r16
    clr FLAGS
    sei
    ret

MOSTRAR:
    push r21
    cpi CONTADOR, 0
    brne NUM1
    ldi r21, 0xC0
    jmp MOSTRAR1

NUM1:
    cpi CONTADOR, 1
    brne NUM2
    ldi r21, 0xF9
    jmp MOSTRAR1

NUM2:
    cpi CONTADOR, 2
    brne NUM3
    ldi r21, 0xA4
    jmp MOSTRAR1

NUM3:
    cpi CONTADOR, 3
    brne NUM4
    ldi r21, 0xB0
    jmp MOSTRAR1

NUM4:
    cpi CONTADOR, 4
    brne NUM5
    ldi r21, 0x99
    jmp MOSTRAR1

NUM5:
    cpi CONTADOR, 5
    brne SAIR
    ldi r21, 0x92
    jmp MOSTRAR1

MOSTRAR1:
    out PORTC, r21

SAIR:
    pop r21
    ret

TIMER0_ISR:
    push r16
    in r16, SREG
    push r16
    dec CONTADOR0
    brne FIM_ISR
    ldi CONTADOR0, 250
    sbr FLAGS, 1 << FLAG_TIMER

FIM_ISR:
    ser r16
    out EIFR, r16
    pop r16
    out SREG, r16
    pop r16
    reti

INT1_ISR:
    push r16
    in r16, SREG
    push r16
    sbr FLAGS, 1 << SW_PARAR
    pop r16
    out SREG, r16
    pop r16
    reti

PISCAR:
    ldi r28, 3

PISCAR1:
    cbr FLAGS, 1 << FLAG_TIMER
    ldi CONTADOR0, 250

PISCAR2:
    sbrs FLAGS, FLAG_TIMER
    jmp PISCAR2
    ldi r16, 0xFF
    out PORTC, r16
    cbr FLAGS, 1 << FLAG_TIMER
    ldi CONTADOR0, 250

PISCAR3:
    sbrs FLAGS, FLAG_TIMER
    jmp PISCAR3
    call MOSTRAR
    dec r28
    brne PISCAR1
    ldi r16, 0b00000000
    out EIMSK, r16
    cbr FLAGS, 1 << SW_PARAR
    ser r16
    out EIFR, r16
    ret

main:
    ldi r16, 0xff
    out spl, r16
    ldi r16, 0x10
    out sph, r16
    call CONFIGURAR

ESPERA_INICIO:
    sbic PIND, SW_INICIO
    jmp ESPERA_INICIO
    ldi CONTADOR, 0
    call MOSTRAR
    cbr FLAGS, 1 << FLAG_TIMER
    ldi CONTADOR0, 250

ATIVA_INT1:
    ldi r16, 0b00000010
    out EIMSK, r16

LOOP_PARAR:
    sbrs FLAGS, SW_PARAR
    jmp LOOP_TIMER
    call PISCAR
    jmp ESPERA_INICIO

LOOP_TIMER:
    sbrs FLAGS, FLAG_TIMER
    jmp LOOP_PARAR
    cbr FLAGS, 1 << FLAG_TIMER
    inc CONTADOR
    cpi CONTADOR, 6
    brne REPETIR
    ldi CONTADOR, 0

REPETIR:
    call MOSTRAR
    jmp LOOP_PARAR

