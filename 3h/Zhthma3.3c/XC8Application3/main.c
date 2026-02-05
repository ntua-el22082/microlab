#define  F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
uint8_t DC_VALUE[17] = {5, 20, 36, 51, 67, 82, 97, 113, 128, 143, 159, 174, 189, 205, 220, 236, 251};
uint8_t input = 0;
uint8_t position=0;
uint8_t mode = 0;   // mode1 -> mode = 0  / mode2 -> mode = 1
uint8_t ADC_H;
uint8_t ADC_L;

int main()
{
	DDRB = 0x02;
	DDRD = 0x00;
	DDRC = 0x00;
	ADMUX = (1 << REFS0) | (1 << ADLAR);  // (1 << REFS0) einai gia na pairnei apo pyknoti, ADLAR = 1 (left adjusted)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // ADEN gia na leitourgei genika o ADC, 111(ADPS) gia 125 kHZ katallhlh syxnothta leitourgias tou ADC (peripou syxnothta metatrophs 125k/13 = 9600 deigmata/sec)
	TCCR1A = (1 << WGM10) |(1 << COM1A1);	// WGM10 kai WGM12 orizoun fast 8-bit PWM me TOP timh ton OCR1A
	TCCR1B = (1 << WGM12) | (1 << CS10);	// CS10 gia clock (sketo), COM1A1 gia paragvgh mh anastrofhs PWM
	position=8;
	OCR1AL = DC_VALUE[position];

	while(1) {
		input = PIND;
		input = ~input;

		// dialexe to mode meso tou PINB

		if(input & 0x01) {
			// an PD0 go to mode1
			mode = 0;
		}
		else if(input & 0x02) {
			// an PD1 go to mode2
			mode = 1;
		}

		// me basi to mode pou eisai ektelese thn katllhl leitourgia

		if(mode == 0) {
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
		else {
			ADCSRA |= (1<<ADSC);	// jekina to conversion
			while (ADCSRA & (1<<ADSC));		// perimene mexri na teleiosei to conversion
			ADC_L = ADCL;
			ADC_H = ADCH;
			OCR1AL = ADC_H;
		}
	}
}