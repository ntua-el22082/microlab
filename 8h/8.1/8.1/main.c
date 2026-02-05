/*
 * main.c
 *
 * Created: 12/8/2025 12:17:23 PM
 *  Author: vkk
 */ 
#include "header.h"
#include <string.h>
volatile uint8_t keypad_interrupt_flag = 0;
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
int main(void)
{	
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
	//	PCA9555_0_write(REG_CONFIGURATION_1, 0xf0); //Set EXT_PORT1 as input/output
	lcd_init();
	lcd_clear_display();
	usart_init(103);
    char response[255];
	while(1)
    {
		 usart_transmit_to_string("ESP:connect\n");
		 usart_receive_string(response);  	
		 if(!strcmp(response,"SUCCESS")){
			 lcd_write_string("1.SUCCESS");
			 }else{
			 lcd_write_string("1.FAIL");
		 }
	_delay_ms(1000);
	lcd_clear_display();
	memset(response, 0, sizeof(response));
	usart_transmit_to_string("ESP:url:\"http://192.168.1.250:5000/data\"\n");
	usart_receive_string(response);
		if(!strcmp(response,"SUCCESS")){
			lcd_write_string("2.SUCCESS");
		}else{
			lcd_write_string("2.FAIL");
		}
	}
	_delay_ms(1000);
	lcd_clear_display();
	 return 0;
}