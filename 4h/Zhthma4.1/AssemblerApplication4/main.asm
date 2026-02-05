.include "m328PBdef.inc" ;ATmega328P microcontroller definition
.equ PD0=0
.equ PD1=1
.equ PD2=2
.equ PD3=3
.equ PD4=4
.equ PD5=5
.equ PD6=6
.equ PD7=7
;.def ADC_L=r18
;.def ADC_H=r19
.org 0x00
rjmp main
.org 0x2A
rjmp ADC_int
.include "definitions.asm"
isolate_digits_16bit:
    ; Clear hundreds counter
    clr r20
    
    ; Load the 16-bit number into working registers
          ; Low byte ZL
            ; High byte ZH
    
hundreds_loop_16:
    ; Compare with 100 (16-bit comparison)
    cpi YL, 100
    ldi r18, 0
    cpc YH, r18
    brlo hundreds_done_16
    
    ; Subtract 100 from 16-bit number
    subi YL, 100
    sbci YH, 0
    inc r20
    rjmp hundreds_loop_16
    
hundreds_done_16:
    ; Now r24 has remainder (0-99), process tens
    clr r21
    
tens_loop_16:
    cpi YL, 10
    brlo tens_done_16
    subi YL, 10
    inc r21
    rjmp tens_loop_16
    
tens_done_16:
    mov r22, YL        ; Ones digit
    ret

ADC_int:; Otan mpoume exei ginei to conversation
rcall lcd_clear_display
;ldi r24, low(1000)
;ldi r25, high(1000)
;rcall wait_msec ; delay 1 Sec
lds ZL,ADCL ; Read ADC result(right adjusted)
lds ZH,ADCH ;
ldi YH,0x00;
ldi YL,0x00;
sbrc ZH,1
adiw Y, 63
sbrc ZH,1
adiw Y, 63
sbrc ZH,1
adiw Y, 63
sbrc ZH,1
adiw Y, 61
sbrc ZH,0
adiw Y, 63
sbrc ZH,0
adiw Y, 62
sbrc ZL,7
adiw Y, 62
sbrc ZL,6
adiw Y, 31
sbrc ZL,5
adiw Y, 16
sbrc ZL,4
adiw Y, 8
sbrc ZL,3
adiw Y, 4
sbrc ZL,2
adiw Y, 2 
sbrc ZL,1
adiw Y ,1
sbrc ZL,0
adiw Y, 1 ;ama goustaroume
rcall isolate_digits_16bit ; ; Output: r20 = hundreds, r21 = tens, r22 = ones
;0x30 einai ta upperbit pou prepei na thesoume gia na epilexoume 
subi r20,-0x30
subi r21,-0x30
subi r22,-0x30
mov r24, r20
call lcd_data
ldi r24,0b00101100
call lcd_data
mov r24, r21
call lcd_data
mov r24, r22
call lcd_data 
ldi r24,' '
call lcd_data
ldi r24,'V' 
call lcd_data
ldi r24,'o'
call lcd_data
ldi r24,'l'
call lcd_data
ldi r24,'t'
call lcd_data
ldi r24,'s'
call lcd_data
ldi r24, low(500)
ldi r25, high(500)
rcall wait_msec 

lds r19, ADCSRA
ori r19, (1<<ADSC)
sts ADCSRA,r19
reti
main:
	ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24           ; init stack pointer
	ser r24
	out DDRD, r24 ; set PORTD as output
	clr r24
    out PORTD, r24         ; Clear PORTD initially
    
    rcall lcd_init
    
    ldi r24, low(100)
    ldi r25, high(100)     ; delay 100 mS
	
    rcall wait_msec
	ldi r16, (1 << REFS0) | (1 << MUX1)|(1 << MUX0)
	sts ADMUX, r16
	ldi r16,(1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADSC) 
	sts ADCSRA, r16
    ; Configure PORTD: PD[7:4] = LCD Data, PD3 = Enable, PD2 = RS, PD[1:0] = unused
	sei
TABLE:
.db 0 , 1, 2, 4, 8 , 16 , 31, 62 , 125 ,250
;2=0,02
; We have made now a decoding to add the values according to the number we have if its 1023 we will add db[0]..db[10] if 6 db[0]..[6]