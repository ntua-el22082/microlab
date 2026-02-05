.include "m328PBdef.inc" ;ATmega328P microcontroller definitions

.def temp = r16
.def ADC_L = r21
.def ADC_H = r22

.org 0x00
 jmp reset
.org 0x2A ;ADC Conversion Complete Interrupt
 reti
reset:
ldi temp, high(RAMEND)
 out SPH,temp
 ldi temp, low(RAMEND)
 out SPL,temp

 ldi temp, 0xFF
 out DDRD, temp ;Set PORTD as output

 ldi temp, 0x00
 out DDRC, temp ;Set PORTC as input

 ; REFSn[1:0]=01 => select Vref=5V, MUXn[4:0]=0000 => select ADC0(pin PC0),
 ; ADLAR=1 => Left adjust the ADC result
 ldi temp, 0b01100000 ;
 sts ADMUX, temp
 ; ADEN=1 => ADC Enable, ADCS=0 => No Conversion,
 ; ADIE=0 => disable adc interrupt, ADPS[2:0]=111 => fADC=16MHz/128=125KHz
 ldi temp, 0b10000111
 sts ADCSRA, temp
Start_conv:
 lds temp, ADCSRA ;
 ori temp, (1<<ADSC) ; Set ADSC flag of ADCSRA
 sts ADCSRA, temp ;
wait_adc:
 lds temp, ADCSRA ;
 sbrc temp,ADSC ; Wait until ADSC flag of ADCSRA becomes 0
 rjmp wait_adc ;

 lds ADC_L,ADCL ; Read ADC result(Left adjusted)
 lds ADC_H,ADCH ;

 out PORTD, ADC_H ; Output ADCH to PORTD

rjmp Start_conv 