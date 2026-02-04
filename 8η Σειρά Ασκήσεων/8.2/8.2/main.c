/*
 * main.c
 *
 * Created: 12/8/2025 12:17:23 PM
 *  Author: vkk
 */ 
#include "header.h"
#include <string.h>
#define ADD_36 12000
/* Routine: usart_init
Description:
This routine initializes the
usart as shown below.
------- INITIALIZATIONS -------
Baud rate: 9600 (Fck= 8MH)
Asynchronous mode
Transmitter on
Reciever on
Communication parameters: 8 Data ,1 Stop, no Parity
--------------------------------
parameters: ubrr to control the BAUD.
return value: None.*/
void usart_init(unsigned int ubrr){
	UCSR0A=0;
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);
	UBRR0H=(unsigned char)(ubrr>>8);
	UBRR0L=(unsigned char)ubrr;
	UCSR0C=(3 << UCSZ00);
	return;
}
/* Routine: usart_transmit
Description:
This routine sends a byte of data
using usart.
parameters:
data: the byte to be transmitted
return value: None. */
void usart_transmit(uint8_t data){
	while(!(UCSR0A&(1<<UDRE0)));// RXCn(positive if data to read) &(is not ready to receive data) 
	UDR0=data;
}
void usart_transmit_to_string(const char* string) {
	while(*string){
		usart_transmit(*string++);
	}
}
/* Routine: usart_receive
Description:
This routine receives a byte of data
from usart.
parameters: None.
return value: the received byte */
uint8_t usart_receive(){
	while(!(UCSR0A&(1<<RXC0)));
	return UDR0;
}void usart_receive_string(char response[]){	memset(response, 0, sizeof(response));	char got;	int i=0;	while(1){	got=usart_receive();	if (got == '\r') {
		continue; // Ignore Carriage Returns (Windows sends \r\n)
	}
	if (got == '\n') {
		break; // Stop receiving on New Line
	}	response[i++]=got;	}	response[i]='\0';/*	lcd_clear_display();	lcd_write_string(response);	_delay_ms(1000);	lcd_clear_display();	*/	return;}
//                     {1 , 2, 4, 8  , 16  32   64  128  256   512 } 
uint16_t kasioumis[10]={20,39,78,156,313,625,1250,2500,5000,10000};   // 20*adc/1024 for ADC MAX 19981 /100
uint16_t converter_table[11]={62,125,250,500,1000,2000,4000,8000,16000,32000,64000};
uint8_t temperature[6];
char full_data[40];
char print[12];
uint8_t *print_celsious;
void buffer_write_string(const char *str,char buff[]){
	while (*str){
		(*buff++)=(*str++);
	}
	*buff='\0';
}
uint32_t final;
void convert_to_celcius()
{
	final=0;
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
	final+=ADD_36;
	print_celsious =  &print[0];
	bcd_real(final, temperature);
	print[1] = temperature[0]+48;
	if(print[1] == '0') {print[1] = ' '; print_celsious=&print[2];}
	print[2] = temperature[1]+48;
	if(print[2] == '0') {print[2] = ' '; print_celsious=&print[3];}
	print[3] = temperature[2]+48;
	print[4] = '.';
	print[5] = temperature[3]+48;
	print[6] = temperature[4]+48;
	print[7] = temperature[5]+48;
	if(read & 0x0001) print[8] = '5';
	else print[8] = '0';
	print[9]='\0';
	return;
}
uint16_t ADC_Read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC)); 
	return ADC;
}
char print_ADC[11];
uint8_t conv_ADC[6];
uint16_t adc_value=0;
void ADC_compute(){
	uint16_t temp=ADC_Read(0);
	adc_value=0;
	for(int i = 0; i<10; ++i){
		if(temp & 0x0001){
			adc_value = adc_value + kasioumis[i];
		}
		temp= temp>>1;
	}
	bcd_real(adc_value,conv_ADC); //hopefully works
//	print_ADC[0]=conv_ADC[0]+48; //will always be zero
	print_ADC[0]=conv_ADC[1]+48;
	print_ADC[1]=conv_ADC[2]+48;
	print_ADC[2]='.';
	print_ADC[3]=conv_ADC[3]+48;
	print_ADC[4]=conv_ADC[4]+48;
	print_ADC[5]=conv_ADC[5]+48;
	print_ADC[6]='\0';
//	print_ADC[6]=conv_ADC[5]+48;
}
char character;
char status[20];
void fix_status(){
				scan_keypad_rising_edge();
				character = keypad_to_ascii(pressed_keys);
				if(character == '0') {
					memset(status, 0, sizeof(status));
					buffer_write_string("NURSE CALL",status);
					}else if(character == '#'){
					if(final > 37000 | final < 34000) {
						buffer_write_string("CHECK TEMPERATURE",status);
						}else if(adc_value > 12000 | adc_value < 4000) {
						buffer_write_string("CHECK PRESSURE",status);
						}else{
						buffer_write_string("OK",status);
					}
				}
		}

int main(void)
{	
	ADMUX = (1 << REFS0); // 5V ref
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
	PCA9555_0_write(REG_CONFIGURATION_1, 0xf0); //Set EXT_PORT1 as input/output
	lcd_init();
	lcd_clear_display();
	usart_init(103);
    char response[255];
	while(1)
    {/*
		 usart_receive_string(response);  	
		 
		 if(!strcmp(response,"SUCCESS")){
			 lcd_write_string("SUCCESS");
			 }else if(!strcmp(response, "FAIL")){
			 lcd_write_string("FAIL");}
		 }*/
	ADC_compute();
	routine();
	convert_to_celcius();//print has our values its an int array 
	fix_status();
	lcd_clear_display();
	buffer_write_string(print_celsious,full_data);
	buffer_write_string(print_ADC,&full_data[8]);
	full_data[7] = ' ';
	lcd_command(0x80);
	lcd_write_string(full_data);
	lcd_command(0xC0);
	lcd_write_string(status);
	_delay_ms(1000);
	}
 return 0;
}