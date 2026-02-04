.include "m328PBdef.inc"
.def intr_counter=r19
.org 0x0
rjmp reset
.org 0x2
rjmp ISR0

.include "wait_x_msec.asm"
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

; Init PORTB as input
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

	ldi YL, 0xe8
	ldi YH, 0x03
	rcall wait_x_msec

	inc r26; increment
	inc r26;increment theloume na xekinaei to pc1-pc5
	andi r26, 0x3f;
	rjmp loop2

ISR0:
	push r25
	push r24
	in r24, SREG ; save r24, r25, SREG
	push r24
	
	in r24, PINB
;   when a button is pressed its zero
	com r24

	andi r24, 0b00011110
	lsr r24

	ldi ZH, high(2*TABLE)
	ldi ZL, low(2*TABLE); word 
	clr r25
	add ZL, r24
	adc ZH, r25
	lpm r24, Z;load program memory 
	out PORTC, r24
	
	pop r24
	out SREG, r24
	pop r24
	pop r25
	reti

TABLE:
	.db 0, 1, 1, 3, 1, 3, 3, 7, 1, 3, 3, 7, 3, 7, 7, 15
	; Index: 4-bit number from PB4-PB1
	; Value: Output for PC0-PC3