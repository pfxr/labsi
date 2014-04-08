#include <avr/io.h>
#include <avr/interrupt.h>
#define FOSC 1000000

 //#define F_CPU 14.7456E6

#define BAUD 2400
#define UBBR_VALUE FOSC/16/BAUD-1
uint8_t cont=0;

void init()
{
 //   DDRD=0b00000000;
    DDRB=0b00000001;

    EICRA=0b00000011;
    EIMSK=0b00000001;
    EIFR=0b00000000;

    UBRR0H=(uint8_t)(UBRR_VALUE>>8);
    UBRR0L=(uint8_t)(UBRR_VALUE);
    UCSR0C=0b00000110;
    UCSR0B=0b00011000;
    UDR0=0;

    SREG=0x80;

}

void envia(uint8_t trama)
{
    while(!(UCSR0A&(1<<UDRE0))){};

    UDR0="0";

}
ISR(INT0_vect)
{
    cont++;
    PORTB^=0x01;
    envia(cont);
}

int main(void)
{
    init();
    while(1)
    {}
    return 0;
}
