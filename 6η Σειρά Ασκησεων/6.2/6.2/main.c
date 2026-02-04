#include "header.h"

#define DELAY_KEYPAD 0x14
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
//char keypad_char[16] = {'D', '#', '0', '*', 'C', '9', '8', '7', 'B', '6', '5', '4', 'A', '3', '2', '1'};
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
int main() {
	char character;
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
	PCA9555_0_write(REG_CONFIGURATION_1, 0xf0); //Set EXT_PORT1 as input/output
	lcd_init();
	lcd_command(0x80);
	while(1) {
		scan_keypad_rising_edge();
		character = keypad_to_ascii(pressed_keys);
		if(character!=0){
		lcd_clear_display();
		lcd_data(character);
		}
		_delay_ms(DELAY_KEYPAD);
	}
}