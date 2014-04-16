#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../nrf24.h"

#define FOSC 1000000

#define F_CPU 1E6

#define BAUD 2400
#define USART_UBBR_VALUE FOSC/16/BAUD-1
volatile int cont=0;
//char buffer_Tx[200];
volatile unsigned char rx;

void init (void)
{
    DDRB|=0x01;
    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8);
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C= 0b00000110;
    UCSR0B= 0b00011000;
    UCSR0B|=0x80;
    nrf_inic();
    SREG |= 0x80;            //Autorizacao global das interrupcoes

}


void enviar(char *Tx)
{
    int i=0;
    while(i<=strlen(Tx))
    {
        while(!(UCSR0A & (1<<UDRE0)));
        UDR0=Tx[i];
        i++;
    }



}

void processar_RX()
{

    char buffer_Tx[4],i;
    char data_array[4];
    switch (rx)
    {
            case '1':
                         do//Espera até que player1 ou player2 enviem '4' (inicio de jogo/disparo)
                        {
                            PORTB|=0x01;

                            if(nrf24_dataReady())
                            {
                                nrf24_getData(data_array);

                            }
                        }while(data_array[0]!='1');
                        if((data_array[0]=='1') && (data_array[1]=='4'))
                        { PORTB^=0x01;
                          sprintf(buffer_Tx,"%c%c",data_array[0],data_array[1]);
                          enviar(buffer_Tx);
                         }
                        for (i=0;i<strlen(buffer_Tx);i++)
                        {
                            data_array[i]=0;
                        }
                        break;

    }


}


ISR (USART_RX_vect)
{
    rx=UDR0;
    processar_RX();
}



int main(void)
{


    init();

    while(1)
    {


    }
}
