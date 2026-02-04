#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
// #include <stdint.h>
uint8_t first=0;
uint8_t high;
uint8_t low_prev=0;
uint8_t low;
uint8_t high_prev=0;
uint8_t volt_[5];
uint8_t ppm_[5];
uint16_t volt; //max 2^16=65536 > 49951
//pinakas megisths akriveias
uint16_t kasioumis[10]={49,98,195,390,781,1563,3125,6250,12500,25000};    //49951 to max reality 4.9951171875 poly komple
uint16_t kasimatis[16]={0,2,3,6,12,25,50,99,198,397,794,1588,3175,6350,12701,25402};//50802 giati auto einai lgk to (2^16-1)/129
char gas[12]  = {'G', 'A', 'S', ' ', 'D', 'E', 'T', 'E', 'C', 'T', 'E', 'D'};//gia 48951 vgazoume 37946 kalo kanonika einai 379.46511 an paroume oti max 4.9551
char clear[5] = {'C', 'L', 'E', 'A', 'R'};									//synolika 379.46644864341085271317829457364 toso tha prepe
uint16_t ppm;

void trela_kolpa(uint8_t len_kas,const uint16_t kas[],uint16_t *value) {
	if(len_kas == 16){
//	uint16_t pointer= *value;
	uint16_t final=0;
		for(int i = 0; i< 16; ++i){
			if(*value & 0x0001){
			final = final + kas[i];
			}
			*value = *value >>1;
		}
	*value=final;
	return;
	}
	high = high & 0x03; // apomonwsh adc
	*value = 0;
	for(int i = 0; i < len_kas; i++) {
		if(i < 8) {
			if(low & 0x01) {
				*value = *value + kas[i];
			}
			low = low >> 1;
		}
		else {
			if(high & 0x01) {
				*value = *value + kas[i];
			}
			high = high >> 1;
		}
	}
	//very important
	//bcd_real(temp); i will do it manually
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

void bcd_real(uint16_t temp, uint8_t conv[]) {
	conv[0] = 0;
	while(temp >= 10000) {
		temp = temp - 10000;
		conv[0]++;
	}
	conv[1] = 0;
	while(temp >= 1000) {
		temp = temp - 1000;
		conv[1]++;
	}
	conv[2] = 0;
	while(temp >= 100) {
		temp = temp - 100;
		conv[2]++;
	}
	conv[3] = 0;
	while(temp >= 10) {
		temp = temp - 10;
		conv[3]++;
	}
	conv[4] = 0;
	while(temp) {		// we don't do it like e = temp because we are afraid of conversion from uint16_t to uint8_t
		temp--;
		conv[4]++;
	}
}

void lcd_message(char argv[], int len) {
	lcd_clear_display();
	lcd_command(0x80);
	for(int i = 0; i<len; i++) {
		lcd_data(argv[i]);
	}
	lcd_command(0xc0);
	
	lcd_data(volt_[0] + 48);
	lcd_data(0b00101100);
	lcd_data(volt_[1] + 48);
	lcd_data(volt_[2] + 48);
	lcd_data(volt_[3] + 48);
	lcd_data(volt_[4] + 48);
	lcd_data('V');
	if(volt<1000){
		ppm_[0]=ppm_[1]=ppm_[2]=ppm_[3]=ppm_[4]=0;
	}
	if(ppm_[0] > 0) {
		lcd_data(ppm_[0] + 48);
	}
	else {
		lcd_data(' ');//ignore 035 we write 35
	}
	if((ppm_[0] > 0) || (ppm_[1] > 0)) {
			lcd_data(ppm_[1] + 48);
		}
		else {
			lcd_data(' ');
	}
	
	lcd_data(ppm_[2] + 48);
	lcd_data(0b00101100);
	lcd_data(ppm_[3] + 48);
	lcd_data(ppm_[4] + 48);
	lcd_data('p');
	lcd_data('p');
	lcd_data('m');
	return;
}



ISR(ADC_vect){
	low = ADCL;
	high = ADCH;
	// C = (V - 1000)/129  >= 129
	// dont forget a + 48 ...
	trela_kolpa(10,kasioumis,&volt);
	//volt=0-50000 a=0-5 ,b=0-9 ,c=0-9
	//volt=100V
	uint8_t port_b = 0x00;
	bcd_real(volt, volt_);
	if(volt_[0]== 4 ) port_b = 0x3f;
	else if(volt_[0] == 4 | (volt_[0] == 3 & volt_[1] > 9)) port_b = 0x1f;
	else if(volt_[0] == 3 & volt_[1] > 2) port_b = 0x0f;
	else if(volt_[0] == 3 | (volt_[0] == 2 & volt_[1] > 5)) port_b = 0x07;
	else if(volt_[0] == 2 | (volt_[0] == 1 & volt_[1] > 8)) port_b = 0x03;
	else if(volt_[0] == 1) port_b = 0x01;
	ppm=(volt-1000);
	uint16_t temporary = ppm;
	trela_kolpa(16,kasimatis,&ppm);
	bcd_real(ppm, ppm_);
	if(temporary>=9675 && volt > 1000){ //129*75=9675
		lcd_message(gas, 12);
		PORTB = (port_b & 0x3f);
		_delay_ms(45);
		PORTB=0x00;
		_delay_ms(45);
		}else{
		lcd_message(clear, 5);
		_delay_ms(90);
		if(ppm > 6000 && ppm <38000) {
			PORTB = 0x01;
		}
		else {
			PORTB = 0x00;
		}
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
	lcd_clear_display();
	_delay_ms(100);
	ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	sei();
	while(1){
		ADCSRA |= (1 << ADSC);
		sei();
		
	}
	return 0;
}

// C = 1 / M * (V - 0.1)
// M = 129 * 100 / 1000 / 1000    M = 129 / 10000
// C = (V - 0.1) * 10000 / 129 >= 75