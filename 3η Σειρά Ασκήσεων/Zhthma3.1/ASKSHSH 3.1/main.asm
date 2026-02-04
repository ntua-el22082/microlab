.include "m328PBdef.inc"

.def DC_VALUE = r20
.def input = r24
.def position = r25

.org 0x00
rjmp main

.include "wait_x_msec.asm"

main:
; ldi YL, 0x01 
; ldi YH, 0x00
ldi ZL, LOW(2*TABLE)
ldi ZH, HIGH(2*TABLE)
ldi position,0x08
adiw Z,0x08

ldi r17, (1 << WGM10) | (1<<COM1A1)
sts TCCR1A, r17
ldi r17, (1<< WGM12) | (1<<CS10)
sts TCCR1B, r17

ldi r17,0x02
out DDRB, r17
; com r17
out PORTB, r17  ; pull up resistors
ser r17				; for debugging purposes
out DDRD, r17		; for debugging purposes

start:
lpm DC_VALUE, Z
sts OCR1AL, DC_VALUE
; rcall wait_x_msec

loop:
in r17,PINB
com r17
mov r18, r17
andi r18,0x08		; PB3 giati to PB5 psilo kollaei (to PB5 douleuei etsi kai etsi) (0x20 gia PINB)
brne meiosi
andi r17,0x10
brne auxisi
result:
lpm DC_VALUE, Z
sts OCR1AL, DC_VALUE
out PORTD, DC_VALUE		; for debugging purposes h vale position
; rcall wait_x_msec
rjmp loop

auxisi:
; rcall wait_x_msec
in r17,PINB
com r17
andi r17,0x10
brne auxisi
cpi position,0x10
breq result
adiw Z,1
inc position
rjmp result

meiosi:
; rcall wait_x_msec
in r17,PINB
com r17
andi r17,0x08
brne meiosi
cpi position,0x00
breq result
sbiw Z,1
dec position
rjmp result

TABLE: .db 5, 20, 36, 51, 67, 82, 97, 113, 128, 143, 159, 174, 189, 205, 220, 236, 251, 0 

; (pososto * 256)
