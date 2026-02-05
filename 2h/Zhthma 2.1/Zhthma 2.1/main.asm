.include "m328PBdef.inc"
.def intr_counter=r19  ; r19 is the INT1 counter, and should not be used in any other way!
  ; high input 
  ; low input byte / little endian

.org 0x0
rjmp reset
.org 0x4
rjmp ISR1

.include "wait_x_msec.asm"
reset:
; Init stack pointer
ldi r24, LOW(RAMEND)
out SPL, r24
ldi r24, HIGH(RAMEND)
out SPH, r24



; Interrupt on rising edge of INT1 pin
ldi r24, (1<<ISC11) | (1<<ISC10)
sts EICRA, r24

; Enable the INT1 interrupt (PD3)
ldi r24, (1<<INT1)
out EIMSK, r24

sei ; Sets the Global Interrupt Flag

; Init PORTB as output
ser r26
out DDRB, r26

; Init PORTC as output
out DDRC, r26

; Init PORTD as input
out PORTD, r26 ; pull-ups of PORTD
clr r26
out DDRD, r26

clr intr_counter ; start with intr_counter = 0


clr r26
loop2:
	out PORTB, r26

	ldi YL, 0xf4
	ldi YH, 0x01
	rcall wait_x_msec

	inc r26

	andi r26, 0x0f;0000 1111
	rjmp loop2

ISR1:
; erwthma b
	push r24
	push r25
	again: ldi r24, (1<<INTF1)
		out EIFR, r24
		ldi YL, 0x05
		ldi YH, 0x00
		rcall wait_x_msec
		in r24, EIFR
		sbrc r24, INTF1 ; Skip the next instruction if INTF1 bit is cleared
		rjmp again

	
	
	in r25, PIND
	; The below should be un-commented if input is of normal logic, and commented if input is of reverse logic
	;com r25
	andi r25, 0b00000010 ; r25: -PD1 otan to pd1 unpressed ouput is 1111 1101 kai me to and 0000 0010 we add 2 if not on its oxff so we add only 0x02

	add intr_counter, r25 ; r19 is the new counter
	
; Checking for upper limit 32 is not necessary (0011 1110 -> 0100 0000 -> ... -> 1111 1110 -> 0000 0000)
	andi r19, 0b00111110
	

	out PORTC, r19

	pop r24
	pop r25
	reti
