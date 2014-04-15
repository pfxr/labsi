/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
*/

#include <avr/io.h>
#include <stdint.h>
#include "../nrf24.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* ------------------------------------------------------------------------- */
/* Software UART routine. */
/* 9600 bps. Transmit only */
/* Transmit pin is: B2 */
/* ------------------------------------------------------------------------- */
/* Hardware specific section ... */
/* ------------------------------------------------------------------------- */
#include <util/delay.h>
#define FOSC 1000000
#define F_CPU 1E6
#define BAUD 2400
#define USART_UBBR_VALUE FOSC/16/BAUD-1
volatile unsigned char rx,flag_rx;
char buffer[30];

/* ------------------------------------------------------------------------- */
/* Printing functions */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
void uart_init()
{
<<<<<<< HEAD:modulos/RX/main.c
    DDRB=0x01;
=======
>>>>>>> e1d6579f3bb6705e512cf049840bcb2accffb266:modulos_emteste/RX/main.c
    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8);
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C= 0b00000110;
    UCSR0B= 0b00011000;
    UCSR0B|=0x80;
    SREG |= 0x80;
}
/* ------------------------------------------------------------------------- */

void enviar(char *Tx)
{
    int i=0;
    while(i<=3)
    {
        while(!(UCSR0A & (1<<UDRE0)));
        UDR0=Tx[i];
        i++;
    }



}

ISR (USART_RX_vect)
{
    rx=UDR0;
    flag_rx=1;
}

void processar_RX()
{
    char buffer_Tx[200];

    if(rx=='1')
    {
        PORTB|=(1<<PB0);
        sprintf(buffer_Tx,"a");
        enviar(buffer_Tx);
    }
    else
        PORTB&=~(1<<PB0);
    flag_rx=0;
}

/* ------------------------------------------------------------------------- */
uint8_t temp;
uint8_t q = 0;
char data_array[4];
uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
/* ------------------------------------------------------------------------- */
int main()
{

    /* init the software uart */
    uart_init();



    /* simple greeting message */
    enviar("\r\n> RX device ready\r\n");

    /* init hardware pins */
    nrf24_init();

    /* Channel #2 , payload length: 4 */
    nrf24_config(2,4);

    /* Set the device addresses */
    nrf24_tx_address(tx_address);
    nrf24_rx_address(rx_address);

    while(1)
    {



            if(nrf24_dataReady())
            {
                nrf24_getData(data_array);
                 enviar(data_array);
        }

    }
}


/* ------------------------------------------------------------------------- */
