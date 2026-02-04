#define  F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
#include <avr/interrupt.h>
uint8_t DC_VALUE[17] = {5, 20, 36, 51, 67, 82, 97, 113, 128, 143, 159, 174, 189, 205, 220, 236, 251};
uint16_t hist[16]={0};
uint8_t input = 0;
uint16_t sum = 0;
uint8_t position=0;
uint8_t be_removed=0;
ISR(TIMER3_COMPA_vect) {
	ADCSRA |= (1<<ADSC);
	while ((ADCSRA & (1<<ADSC)) != 0);
	uint16_t curr = ADC;
	sum += curr;
	sum -= hist[be_removed]; //afairoume auto pou deixnei o deikths
	hist[be_removed++] = curr;
	be_removed&=0x000f;	//0x10 && 0f
	
	curr = sum >>4;
	
	uint8_t input;
	
	if (curr <= 200) {
		input = 0x01;
	}else
	if ( curr <= 400) {
		input = 0x02;
	}else
	if (curr <= 600) {
		input = 0x04;
	}else
	if ( curr <= 800) {
		input = 0x08;
		}else{
		input=0x10;
	}
	//input&=0x1F;
	PORTD = input;
}
int main()
{
	DDRB = 0x02;
	DDRD = 0xff;
	DDRC = 0x00;
	ADMUX = (1<<REFS0) | (1<<MUX0);    // ADLAR = 0 (right adjusted)
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);//
	TCCR1A = (1 << WGM10) |(1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1 << CS10);
	position=8;
	TCCR3A = 0x00;
	TCCR3B = (0<<WGM33) | (1<<WGM32) | (0<<CS32) | (1<<CS31) | (1<<CS30);// WGM32==1 tote otan h sygkrish einai epityxhs me ton OCR3A mhdenizetai o timer 011 clk/64
	// 16MHz / (64 * 10Hz) - 1 sel 121 10hz=100msec
	OCR3A = 24999;
	TIMSK3 = (1<<OCIE3A);// ekteleitai alma sto dianysma diakophs tou timer3_compare
	OCR1AL = DC_VALUE[position];
	sei();
	while(1) {
		input = PINB;
		input = ~input;
		// complement of 1
		if((input & 0x10) == 0x10) {
			// an PB4 auxise to led
			while((~(input = PINB) & 0x10) == 0x10) { _delay_ms(1); }
			if(position != 0x10) { position++;}
		}
		else if((input & 0x08) == 0x08) {   // an PB3 meiose to LED
			while((~(input = PINB) & 0x08) == 0x08) { _delay_ms(1) ;  }
			if((position != 0x00)) { position--;}
		}
		OCR1AL = DC_VALUE[position];

	}
}