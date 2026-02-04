#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
#include <avr/interrupt.h>
// #include <stdint.h>
uint8_t high;
uint8_t low;
uint8_t a = 0, b = 0, c = 0;
uint8_t kasioumis[10] = {1, 1, 2, 4, 8, 16, 31, 62, 125, 250};
void write_2_nibbles(uint8_t word) {
	uint8_t input;
	uint8_t output;
	input = PIND;
	output = (input & 0x0f) | (word & 0xf0);
	output |=word & 0xf0;
	PORTD = output;
	PORTD |= 0b00001000;    // sbi
asm volatile("nop\n\t"
"nop\n\t");
	PORTD &= 0b11110111;    // cbi
	output = (input & 0x0f) | ((word & 0x0f) << 4);
	PORTD=output;
	PORTD |= 0b00001000;
asm volatile("nop\n\t"
"nop\n\t");
	PORTD &= 0b11110111;
	return;
}

void lcd_data(uint8_t word) {
	PORTD |= 0b00000100;
	write_2_nibbles(word);
	_delay_us(250);
	return;
}

void lcd_command(uint8_t command) {
	PORTD &= 0b11111011;
	write_2_nibbles(command);
	_delay_us(250);
	return;
}

void lcd_clear_display() {
	lcd_command(0x01);
	_delay_ms(5);
	return;
}

void lcd_init() {
	_delay_ms(200);
	for(int i=0; i<3; i++) {
		PORTD = 0x30;
		PORTD |= 0b00001000;
		;
		;
		PORTD &= 0b11110111;
		_delay_us(250);
	}
	PORTD = 0x20;
	PORTD |= 0b00001000;
	;
	;
	PORTD &= 0b11110111;
	_delay_us(250);
	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_clear_display();
	lcd_command(0x06);
	return;
}

void bcd_conversion() {
	high = high & 0x03;
	uint16_t temp = 0;
	for(int i = 0; i < 10; i++) {
		if(i < 8) {
			if(low & 0x01) {
				temp = temp + kasioumis[i];
			}
			low = low >> 1;
		}
		else {
			if(high & 0x01) {
				temp = temp + kasioumis[i];
			}
			high = high >> 1;
		}
	}
	a = 0;
	while(temp >= 100) {
		temp = temp - 100;
		a++;
	}
	b = 0;
	while(temp >= 10) {
		temp = temp - 10;
		b++;
	}
	c = 0;
	while(temp) {		// we don't do it like c = temp because we are afraid of conversion from uint16_t to uint8_t
		temp--;
		c++;
	}
}

int main(void)
{
	DDRD = 0xff;
	PORTD = 0x00;
	lcd_init();
	_delay_ms(100);
	ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	while(1) {
		_delay_ms(500);
		lcd_clear_display();
		ADCSRA |= (1 << ADSC);
		while(ADCSRA & (1 << ADSC)) {
			;
		}
		low = ADCL;
		high = ADCH;
		bcd_conversion();
		a = a + 48;
		b = b + 48;
		c = c + 48;
		lcd_data(a);
		lcd_data(44);
		lcd_data(b);
		lcd_data(c);
		_delay_ms(500);
	}
}