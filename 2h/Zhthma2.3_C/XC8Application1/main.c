#define F_CPU 16000000UL
#include <xc.h>
#include <avr/interrupt.h>
#include <util/delay.h>



volatile uint16_t count;
volatile uint8_t first;

ISR (INT1_vect)
{
	do {
		EIFR = (1<<INTF1);
		_delay_ms(5);  //spinthirismos
	} while((EIFR & 0x02)!=0);
	first = 0;
	if((count > 100) && (count < 4001)) {
		first = 1;
	}
	count = 4000;

	return;
}

int main(void)
{
	count = 0;
	first = 0;
	EICRA=(1 << ISC11) | (1 << ISC10); //interrupt on rising edge of INT01
	EIMSK=(1 << INT1); //enable the INT1 interrupt (PD3)
	sei();//enable interrupts
	DDRB=0xFF; //Set PORTB as output
	DDRD=0x00; //Set PORTD as input
	//PORTD=0xFF; //enable pull-up resistors in PORTD paixe mexri na paixei swsta
	PORTB=0x00;
	while(1)
	{
		if(count > 0) {
			if(count >= 3000) {
				if(first == 1) {
					//cli();
					PORTB = 0xff;
					//sei();
					}else{
					//cli();
					PORTB = 0x08;
					first=0;
					//sei();
				}
			}
			else{
				//cli();
				PORTB = 0x08;
				first=0;
				//sei();
			}
			//cli();
			_delay_ms(1);
			//sei();
			if(count > 0) {
				count = count - 1;
			}
		}
		else {
			//cli();
			PORTB = 0x00;
			//    first=0;
			//sei();
		}
	}
}