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

void uart_init()
{

    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8);
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C= 0b00000110;
    UCSR0B= 0b00011000;
    UCSR0B|=0x80;
    SREG |= 0x80;
}

void enviar(char *Tx)
{
    int i=0;

    //strcat(Tx,"<");
    while(i<=strlen(Tx))
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


uint8_t temp;
uint8_t q = 0;
uint8_t data_array[4];
uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
/* ------------------------------------------------------------------------- */
int main()
{
    /* init the software uart */
    uart_init();

    /* init the xprintf library */
    xdev_out(uart_put_char);

    /* simple greeting message */
    xprintf("\r\n> TX device ready\r\n");
    
    /* init hardware pins */
    nrf24_init();
    
    /* Channel #2 , payload length: 4 */
    nrf24_config(2,4);

    /* Set the device addresses */
    nrf24_tx_address(tx_address);
    nrf24_rx_address(rx_address);    

    while(1)
    {                
        /* Fill the data buffer */
        data_array[0] = 0x00;
        data_array[1] = 0xAA;
        data_array[2] = 0x55;
        data_array[3] = q++;                                    

        /* Automatically goes to TX mode */
        nrf24_send(data_array);        
        
        /* Wait for transmission to end */
        while(nrf24_isSending());

        /* Make analysis on last tranmission attempt */
        temp = nrf24_lastMessageStatus();

        if(temp == NRF24_TRANSMISSON_OK)
        {                    
            xprintf("> Tranmission went OK\r\n");
        }
        else if(temp == NRF24_MESSAGE_LOST)
        {                    
            xprintf("> Message is lost ...\r\n");    
        }
        
		/* Retranmission count indicates the tranmission quality */
		temp = nrf24_retransmissionCount();
		xprintf("> Retranmission count: %d\r\n",temp);

		/* Optionally, go back to RX mode ... */
		nrf24_powerUpRx();

		/* Or you might want to power down after TX */
		// nrf24_powerDown();            

		/* Wait a little ... */
		_delay_ms(10);
    }
}
/* ------------------------------------------------------------------------- */