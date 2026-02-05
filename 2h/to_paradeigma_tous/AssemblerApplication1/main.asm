.include "m328PBdef.inc"

.equ FOSC_MHZ=16 ; Microcontroller operating frequency in MHz
.equ DEL_mS=500 ; Delay in mS (valid number from 1 to 4095)
.equ DEL_NU=FOSC_MHZ*DEL_mS; delay_mS routine: (1000*DEL_NU+6) cycles
.def intr_counter=r19  ; r19 is the INT1 counter, and should not be used in any other way!
;.DEF AH=r29  ; high input byte
;.DEF AL=r28  ; low input byte / little endian

.org 0x0
rjmp reset
.org 0x2
rjmp ISR0

.include "Yreg_wait_x_msec.asm"
reset:
; Init stack pointer
ldi r24, LOW(RAMEND)
out SPL, r24
ldi r24, HIGH(RAMEND)
out SPH, r24

; Interrupts on rising edge of INT0, INT1 pin
ldi r24, (1<<ISC11) | (1<<ISC10) | (1<<ISC01) | (1<<ISC00)
sts EICRA, r24

; Enable INT0, INT1 interrupt (PD2, PD3 respectively)
ldi r24, (1<<INT1) | (1<<INT0)
out EIMSK, r24

sei ; Sets the Global Interrupt Flag

; Init PORTB as output
clr r26
out DDRB, r26
ser r26
out PORTB, r26 ; pull-ups of PORTB

; Init PORTC as output
out DDRC, r26

; Init PORTD as input
clr r26
out DDRD, r26
ser r26
out PORTD, r26 ; pull-ups of PORTD

clr intr_counter ; start with intr_counter = 0

loop1:
	clr r26
loop2:
	out PORTC, r26

	ldi YL, LOW(1000)
	ldi YH, HIGH(1000)
	rcall wait_x_msec

	inc r26
	inc r26

	cpi r26, 64
	breq loop1
	rjmp loop2

ISR0:
	push r25
	push r24
	in r24, SREG ; save r24, r25, SREG
	push r24
	
	in r24, PINB
; The below should be commented if input is of NORMAL logic
	com r24

	andi r24, 0b00011110
	lsr r24

	ldi ZH, high(2*LUT)
	ldi ZL, low(2*LUT)
	clr r25
	add ZL, r24
	adc ZH, r25
	lpm r24, Z
	
	;in r25, PORTC
	;or r24, r25 ; We mux the two outputs (counter and isr output) so that the counting doesn't stop being displayed every time
				; If we didn't do that, every time isr output, all other MSB's would be cleared
	out PORTC, r24
	
	pop r24
	out SREG, r24
	pop r24
	pop r25
	reti

LUT:
	.db 0, 1, 1, 3, 1, 3, 3, 7, 1, 3, 3, 7, 3, 7, 7, 15
	; Index: 4-bit number from PB4-PB1
	; Value: Output for PC0-PC3