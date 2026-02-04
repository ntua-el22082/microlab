#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
#include <avr/interrupt.h>
// #include <stdint.h>
uint8_t high;
uint8_t low;
uint8_t a = 0, b = 0, c = 0 ;
uint16_t volt;
uint8_t kasioumis[10] = {1, 1, 2, 4, 8, 16, 31, 62, 125, 250};
char gas[12]  = {'G', 'A', 'S', ' ', 'D', 'E', 'T', 'E', 'C', 'T', 'E', 'D'};
char clear[5] = {'C', 'L', 'E', 'A', 'R'};
uint16_t real_adc;
uint16_t ppm;

void ppm_() {
	ppm = 100 * (volt - 10);
	ppm = ppm >> 7;  // /128 anti gia 129
	if(volt > 333) { ppm = ppm - 2;}
	else if(volt > 166) { ppm = ppm - 1;}
//	else if(volt > 110) {ppm = ppm - 1;}  // kanonika einai 66 apla to kaname gia na einai akribos sto 75 otan v = 1.07, einai thema akribeias
	if(ppm > 395) ppm = 0;
	
}

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

void bcd_real(uint16_t temp) {
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

void lcd_message(char argv[], int len) {
	lcd_command(0x80);
	for(int i = 0; i<len; i++) {
		lcd_data(argv[i]);
	}
	lcd_command(0xc0);
	lcd_data(a + 48);
	lcd_data(0b00101100);
	lcd_data(b + 48);
	lcd_data(c + 48);
	lcd_data(' ');
	lcd_data('V');
	lcd_data(' ');
	ppm_();
	bcd_real(ppm);
	if(a > 0) {
		lcd_data(a + 48);
	}
	else {
		lcd_data(' ');
	}
	lcd_data(b + 48);
	lcd_data(c + 48);
	lcd_data(' ');
	lcd_data('p');
	lcd_data('p');
	lcd_data('m');
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
	volt=temp; //very important
	bcd_real(temp);
}

ISR(ADC_vect){
	lcd_clear_display();
	low = ADCL;
	high = ADCH;
	// C = (100*V - 10) * 4 >= 129*3
	// dont forget a + 48 ...
	bcd_conversion();
	//volt=0-500 a=0-5 ,b=0-9 ,c=0-9
	//volt=100V
	uint8_t port_b = 0x00;
	if(a == 5 | (a == 4 & b > 5)) port_b = 0x3f;
	else if(a == 4 | (a == 3 & b > 9)) port_b = 0x1f;
	else if(a == 3 & b > 2) port_b = 0x0f;
	else if(a == 3 | (a == 2 & b > 5)) port_b = 0x07;
	else if(a == 2 | (a == 1 & b > 8)) port_b = 0x03;
	else if(a == 1) port_b = 0x01;
	uint16_t C_x=0;
	C_x=(volt-10) << 2;
	if(C_x>=387 && volt > 10){ //129*3=387
		lcd_message(gas, 12);
		PORTB = (port_b & 0x3f);
		}else{
		lcd_message(clear, 5);
		PORTB = 0x00;
	}
	return;
}

int main()
{
	DDRD = 0xff;
	DDRB = 0xff;
	PORTB = 0x00;
	PORTD = 0x00;
	lcd_init();
	_delay_ms(100);
	ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	sei();
	while(1){
		_delay_ms(95);
		ADCSRA |= (1 << ADSC);
		sei();
	}
	return 0;
}

// C = 1 / M * (V - 0.1)
// M = 129 * 100 / 1000 / 1000    M = 129 / 10000
// C = (V - 0.1) * 10000 / 129 >= 75