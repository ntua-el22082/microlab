.include "m328PBdef.inc"

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
; Enable the INT1 interrupt (PD3)
ldi r24, (1<<INT1)
out EIMSK, r24
; Interrupt on rising edge of INT1 pin
ldi r24, (1<<ISC11) | (1<<ISC10)
sts EICRA, r24
sei ; Sets the Global Interrupt Flag
; Init PORTB as output and PORTD as input
ser r26
out DDRB, r26
out PORTD, r26 ; pull-ups of PORTD
clr r26
out DDRD, r26;
loop1:
	nop
	rjmp loop1
ISR1:
	push YL
	push YH
	in YH, SREG ; save r24, r25, SREG
	push YH
	push r24
	push r26 
	again: ldi r24, (1<<INTF1)
		out EIFR, r24
		ldi YL, 0x05
		ldi YH, 0x00
		rcall wait_x_msec
		in r24, EIFR
		sbrc r24, INTF1 ; Skip the next instruction if INTF1 bit is cleared
		rjmp again
	sei 
	brtc main_prog; branch if T is clear
	renewal:
		ser r24
		out PORTB,r24; turn on all the lights PB0-PB5
		ldi YH,0x03
		ldi YL,0xe8; 0x3e8=1000 msec
		rcall wait_x_msec
		ldi r24, 0x8; 0000 1000 PB3
		out PORTB, r24; turn on PB3
		ldi YH,0x0b
		ldi YL,0xb8; 3000=0x0bb8
		rcall wait_x_msec
		clr r24 ;r24=0x00
		out PORTB,r24 ;turn the leds off
		pop YL
		pop YH
		out SREG, YH
		pop YH
		pop r24 
		pop	r26
		clt ;clear to T flag
		reti
	main_prog:
		set ; flag T=1
		ldi r24, 0x8;
		out PORTB, r24;
		ldi YL ,0xa0;
		ldi YH ,0x0f;0x0fa0=4000msec
		rcall wait_x_msec
		clr r24
		out PORTB, r24;clear output
		pop YL
		pop YH
		out SREG, YH
		pop YH
		pop r24 
		pop	r26
		clt
		reti


	
