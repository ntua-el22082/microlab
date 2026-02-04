#include "header.h"
#define DELAY_KEYPAD 35
uint16_t pressed_keys = 0x0000;


uint16_t scan_row(uint8_t row) {	// row = {0, 1, 2, 3}
	uint8_t scan = 1 << row;	// 0b00000100 if row = 2
	scan = scan ^ 0x0f;		// 0b00001011
	PCA9555_0_write(REG_OUTPUT_1, scan);
	uint8_t input = PCA9555_0_read(REG_INPUT_1);
	input = ~input;
	input = input >> 4;
	uint16_t return_value = (uint16_t) (input & 0x0f);
	return_value = return_value << (4*row);		// kratame olh thn eisodo
	return return_value;
}

uint16_t scan_keypad() {
	uint16_t keypad = 0;
	for(uint8_t row_to_scan = 0; row_to_scan < 4; row_to_scan++) {
		keypad |= scan_row(row_to_scan);
	}
	return keypad;
}
void scan_keypad_rising_edge() {
	uint16_t pressed_keys_tempo = scan_keypad();
	_delay_ms(DELAY_KEYPAD);
	uint16_t pressed_keys_tempo_b = scan_keypad();
	pressed_keys_tempo &= pressed_keys_tempo_b;
	pressed_keys = pressed_keys_tempo;
}
char keypad_char[16] = {'*','0', '#', 'D', '7', '8', '9', 'C', '4', '5', '6', 'B', '1' , '2', '3','A'};

char keypad_to_ascii(uint16_t input) {	// 0b1101000000001001
	uint8_t count = 0;
	while(!(input & 0x0001)) {
		input = input >> 1;
		count++;
		if(count == 16) return 0;	// null
	}
	char result = keypad_char[count];
	return result;
}

void unlock_lock() {
	lcd_command(0x80);
	lcd_write_string("LOCK UNLOCKED");
	lcd_command(0xC0);
	lcd_write_string("Ma anoixes");
	PORTB = 0x3f;
	_delay_ms(3000);
	PORTB = 0x00;
	lcd_clear_display();
	_delay_ms(2000);
}

void lock_lock() {
	lcd_command(0x80);
	lcd_write_string("WRONG CODE");
	lcd_command(0xC0);
	lcd_write_string("Mhn kleveis klefth");
	for(uint8_t i=0; i<6; i++) {
		PORTB = 0x3f;
		_delay_ms(500);
		PORTB = 0x00;
		_delay_ms(500);
	}
	lcd_clear_display();
}

int main() {
	DDRB = 0xff;
	PORTB = 0x00;
	char character = 0;
	char previous_character = 0;
	uint8_t index = 0;
	char answer[2] = {0, 0};
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
	PCA9555_0_write(REG_CONFIGURATION_1, 0xf0); //Set EXT_PORT1 as input/output
	lcd_init();
	while(1) {
		scan_keypad_rising_edge();
		character = keypad_to_ascii(pressed_keys);
		if((character != previous_character) & (character != 0)) {
			answer[index] = character;
			index++;
			if(index == 2) {
				index = 0;
				if((answer[0] == '4') & (answer[1] == '0')) {
					unlock_lock();
					previous_character=0xFF;
					_delay_ms(DELAY_KEYPAD);
					continue;
				}
				else {
					lock_lock();
					previous_character=0xFF;
					_delay_ms(DELAY_KEYPAD);
					continue;
				}
			}
		}
		previous_character = character;
		_delay_ms(DELAY_KEYPAD);
	}
}