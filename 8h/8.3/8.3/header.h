#define F_CPU 16000000UL
#include <xc.h>
#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#define PCA9555_0_ADDRESS 0x40 //A0=A1=A2=0 by hardware
#define TWI_READ 1 // reading from twi device
#define TWI_WRITE 0 // writing to twi device
#define SCL_CLOCK 100000L // twi clock in Hz
//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2
// PCA9555 REGISTERS
typedef enum {
	REG_INPUT_0 = 0,
	REG_INPUT_1 = 1,
	REG_OUTPUT_0 = 2,
	REG_OUTPUT_1 = 3,
	REG_POLARITY_INV_0 = 4,
	REG_POLARITY_INV_1 = 5,
	REG_CONFIGURATION_0 = 6,
	REG_CONFIGURATION_1 = 7
} PCA9555_REGISTERS;
//----------- Master Transmitter/Receiver -------------------
#define TW_START 0x08
#define TW_REP_START 0x10
//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 0x18
#define TW_MT_SLA_NACK 0x20
#define TW_MT_DATA_ACK 0x28
//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK 0x40
#define TW_MR_SLA_NACK 0x48
#define TW_MR_DATA_NACK 0x58
#define TW_STATUS_MASK 0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)
uint16_t read;
uint8_t lcd_read = 0x00;
//initialize TWI clock
void twi_init(void)
{
	TWSR0 = 0; // PRESCALER_VALUE=1
	TWBR0 = TWBR0_VALUE; // SCL_CLOCK 100KHz
}
// Read one byte from the twi device (request more data from device)
unsigned char twi_readAck(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR0 & (1<<TWINT)));
	return TWDR0;
}
//Read one byte from the twi device, read is followed by a stop condition
unsigned char twi_readNak(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR0 & (1<<TWINT)));
	return TWDR0;
}
// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1= failed to access device
unsigned char twi_start(unsigned char address)
{
	uint8_t twi_status;
	// send START condition
	TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));
	// check value of TWI Status Register.
	twi_status = TW_STATUS & 0xF8;
	if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) return 1;
	// send device address
	TWDR0 = address;
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR0 & (1<<TWINT)));
	// check value of TWI Status Register.
	twi_status = TW_STATUS & 0xF8;
	if ( (twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK) )
	{
		return 1;
	}
	return 0;
}
// Send start condition, address, transfer direction.
// Use ack polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
	uint8_t twi_status;
	while ( 1 )
	{
		// send START condition
		TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

		// wait until transmission completed
		while(!(TWCR0 & (1<<TWINT)));

		// check value of TWI Status Register.
		twi_status = TW_STATUS & 0xF8;
		if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) continue;

		// send device address
		TWDR0 = address;
		TWCR0 = (1<<TWINT) | (1<<TWEN);

		// wail until transmission completed
		while(!(TWCR0 & (1<<TWINT)));

		// check value of TWI Status Register.
		twi_status = TW_STATUS & 0xF8;
		if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK) )
		{
			/* device busy, send stop condition to terminate write operation */
			TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

			// wait until stop condition is executed and bus released
			while(TWCR0 & (1<<TWSTO));

			continue;
		}
		break;
	}
}
// Send one byte to twi device, Return 0 if write successful or 1 if write failed
unsigned char twi_write( unsigned char data )
{
	// send data to the previously addressed device
	TWDR0 = data;
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));
	if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1;
	return 0;
}
// Send repeated start condition, address, transfer direction
//Return: 0 device accessible
// 1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
	return twi_start( address );
}
// Terminates the data transfer and releases the twi bus
void twi_stop(void)
{
	// send stop condition
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	// wait until stop condition is executed and bus released
	while(TWCR0 & (1<<TWSTO));
}
void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
	twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
	twi_write(reg);
	twi_write(value);
	twi_stop();
}
uint8_t PCA9555_0_read(PCA9555_REGISTERS reg)
{
	uint8_t ret_val;

	twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
	twi_write(reg);
	twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
	ret_val = twi_readNak();
	twi_stop();
	return ret_val;
}
void write_2_nibbles(uint8_t word) {
	uint8_t input;
	uint8_t output;
	input = lcd_read;
	output = (input & 0x0f) | (word & 0xf0);
	output |=word & 0xf0;
	PCA9555_0_write(REG_OUTPUT_0, output);
	output |= 0b00001000;
	PCA9555_0_write(REG_OUTPUT_0, output);		// sbi
	//	PORTD |= 0b00001000;    // sbi
	asm volatile("nop\n\t"
	"nop\n\t");
	output &= 0b11110111;
	PCA9555_0_write(REG_OUTPUT_0, output);
	//	PORTD &= 0b11110111;    // cbi
	output = (input & 0x0f) | ((word & 0x0f) << 4);
	PCA9555_0_write(REG_OUTPUT_0, output);
	//	PORTD=output;
	output |= 0b00001000;
	PCA9555_0_write(REG_OUTPUT_0, output);
	//	PORTD |= 0b00001000;
	asm volatile("nop\n\t"
	"nop\n\t");
	output &= 0b11110111;
	PCA9555_0_write(REG_OUTPUT_0, output);
	lcd_read = output;
	//	PORTD &= 0b11110111;
	return;
}

void lcd_data(uint8_t word) {
	//	PCA9555_0_write(REG_OUTPUT_0, 0b00000100);
	lcd_read |= 0b00000100;
	PCA9555_0_write(REG_OUTPUT_0, lcd_read);
	//	PORTD |= 0b00000100;
	write_2_nibbles(word);
	_delay_us(250);
	return;
}

void lcd_command(uint8_t command) {
	lcd_read &= 0b11111011;
	PCA9555_0_write(REG_OUTPUT_0, lcd_read);
	//	PORTD &= 0b11111011;
	write_2_nibbles(command);
	_delay_us(250);
	return;
}

void lcd_clear_display() {
	lcd_command(0x01);
	lcd_read = 0x01;
	_delay_ms(5);
	return;
}

void lcd_init() {
	_delay_ms(200);
	for(int i=0; i<3; i++) {
		//		PORTD = 0x30;
		lcd_read = 0x30;
		PCA9555_0_write(REG_OUTPUT_0, lcd_read);
		lcd_read |= 0b00001000;
		PCA9555_0_write(REG_OUTPUT_0, lcd_read);
		//		PORTD |= 0b00001000;
		;
		;
		lcd_read &= 0b11110111;
		PCA9555_0_write(REG_OUTPUT_0, lcd_read);
		//		PORTD &= 0b11110111;
		_delay_us(250);
	}
	lcd_read = 0x20;
	PCA9555_0_write(REG_OUTPUT_0, lcd_read);
	//	PORTD = 0x20;
	lcd_read |= 0b00001000;
	PCA9555_0_write(REG_OUTPUT_0, lcd_read);
	//	PORTD |= 0b00001000;
	;
	;
	lcd_read &= 0b11110111;
	PCA9555_0_write(REG_OUTPUT_0, lcd_read);
	//	PORTD &= 0b11110111;
	_delay_us(250);
	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_clear_display();
	lcd_command(0x06);
	return;
}
void lcd_write_string(const char *str){
	while (*str){
		lcd_data(*str++);
	}
}
int one_wire_reset(void){
	uint8_t temp;
	DDRD |=0x10;  // sbi DDRD, PD4
	PORTD &= 0xEF; //cbi PORTD, PD4
	_delay_us(480);
	DDRD  &= 0xEF; //cbi DDRD, PD4
	PORTD &= 0xEF; //cbi PORTD, PD4
	_delay_us(100);
	temp=PIND;
	_delay_us(380);
	temp &=0x10;
	return (temp==0); // if a connected device is detected(PD4=0) return 1
}
int one_wire_receive_bit(void){
	uint8_t temp=0x10;
	DDRD |=0x10;
	PORTD &= 0xEF;
	_delay_us(2);
	DDRD &= 0xEF;
	PORTD &= 0xEF;
	_delay_us(10);
	temp &= PIND;
	_delay_us(49);
	return (temp & 0x10) != 0;
}
void one_wire_transmit_bit(uint8_t output){
	output &=0x10;
	DDRD |=0x10;   // sbi DDRD, PD4
	PORTD &= 0xEF;
	_delay_us(2);
	PORTD |=output;
	_delay_us(58);
	DDRD  &= 0xEF; //cbi DDRD, PD4
	PORTD &= 0xEF; //cbi PORTD, PD4
	_delay_us(1);
	return ;
}
int one_wire_receive_byte(){
	uint8_t temp=0;
	//	uint8_t result;
	for(int i=0; i<8; ++i){
		temp >>= 1;
		if (one_wire_receive_bit()) {
			temp |= 0x80;
		}
	}
	return temp;
}
void one_wire_transmit_byte(uint8_t byte){
	for(int i=0; i<8; ++i){
		// -1 in binary is 0xFF, 0 becomes 0x00.
		one_wire_transmit_bit( -(byte & 0x01) );
		byte >>= 1;
	}
}
//1550000
//akriveia 3 dekadikwn == *1000
void bcd_real(uint32_t temp, uint8_t conv[]) {
	conv[0] = 0;
	while(temp >= 100000) {
		temp = temp - 100000;
		conv[0]++;
	}
	conv[1] = 0;
	while(temp >= 10000) {
		temp = temp - 10000;
		conv[1]++;
	}
	conv[2] = 0;
	while(temp >= 1000) {
		temp = temp - 1000;
		conv[2]++;
	}
	conv[3] = 0;
	while(temp >= 100) {
		temp = temp - 100;
		conv[3]++;
	}
	conv[4]=0;
	while(temp >= 10) {
		temp = temp - 10;
		conv[4]++;
	}
	conv[5]=0;
	while(temp) {		// we don't do it like e = temp because we are afraid of conversion from uint16_t to uint8_t
		temp--;
		conv[5]++;
	}
	
}

uint16_t routine(/*uint16_t read exei bei global */){
	read=0;
	if(!one_wire_reset()){
		return 0x8000;
	}
	one_wire_transmit_byte(0xCC);
	one_wire_transmit_byte(0x44);
	while(!one_wire_receive_bit());
	one_wire_reset();
	one_wire_transmit_byte(0xCC);
	one_wire_transmit_byte(0xBE);
	read=one_wire_receive_byte();
	read|=one_wire_receive_byte()<<8;
	return 0; //all good
}
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
