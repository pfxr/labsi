#include<avr/io.h>
#include<avr/interrupt.h>
#include "nrf24.h"
//volatile char buff[4];
void init()
{
    inic_nrf();
    MCUCR=0b00000011;
    GICR=0b01000001;
    DDRB=0x01;

    SREG=0x80;

}

ISR(INT0_vect)
{
    enviar_nrf("14");
    PORTB^=0x01;

}
int main()
{
    init();
    while(1)
    {

    }
}
