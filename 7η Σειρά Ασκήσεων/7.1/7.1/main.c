#include "header.h"
uint16_t read;
char print[10];
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

uint16_t converter_table[11]={62,125,250,500,1000,2000,4000,8000,16000,32000,64000};
uint8_t temperature[6];
void convert_to_celcius()
{
	uint32_t final=0;
	uint16_t temp = read;
	if((temp & 0xF800)==0xF800){
		print[0]='-';
	}else{
		print[0]=' ';
	}
	for(int i = 0; i<11; ++i){
		if(temp & 0x0001){
			final = final + (uint32_t)converter_table[i];
		}
		temp= temp>>1;
	}
	bcd_real(final, temperature);
	print[1] = temperature[0]+48;
	if(print[1] == '0') print[1] = ' ';
	print[2] = temperature[1]+48;
	if(print[2] == '0') print[2] = ' ';
	print[3] = temperature[2]+48;
	print[4] = '.';
	print[5] = temperature[3]+48;
	print[6] = temperature[4]+48;
	print[7] = temperature[5]+48;
	if(read & 0x0001) print[8] = '5';
	else print[8] = '0';
	for(int i = 0; i<9; i++) {
		lcd_data(print[i]);
	}	
	lcd_data(0xDF);
	lcd_data('C');
	return;
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
int main(void)
{	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
//	PCA9555_0_write(REG_CONFIGURATION_1, 0xf0); //Set EXT_PORT1 as input/output
	lcd_init();
	while(1){
	int	temp=routine();
		if(temp==0x8000){
			lcd_write_string("NO Device");
			_delay_ms(1000);
			lcd_clear_display();
			continue;
		}
	
		convert_to_celcius();
		_delay_ms(1000);
		lcd_clear_display();
	}
	
}