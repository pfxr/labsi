#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../nrf24.h"

#define pedro1 {0xD7,0xD7,0xD7,0xD7,0xD7}
#define pedro2 {0xE7,0xE7,0xE7,0xE7,0xE7}
#define joao1  {0xA7,0xA7,0xA7,0xA7,0xA7}
#define joao2  {0xB7,0xB7,0xB7,0xB7,0xB7}

#define FOSC 1000000
#define F_CPU 1000000
#define BAUD 2400
#define USART_UBBR_VALUE FOSC/16/BAUD-1

//char buffer_Tx[200];
volatile unsigned char rx;

char data_array[4],cont_players=0,buffer[40];;

uint8_t rx_address[5] = joao1;
uint8_t tx_address[5] = pedro1;

void init (void)
{
    DDRB|=0x01;
    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8);
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C= 0b00000110;
    UCSR0B= 0b00011000;
    UCSR0B|=0x80;

    SREG |= 0x80;			//Autorizacao global das interrupcoes

}
void nrf_receber()
{
    if(nrf24_dataReady())
    {
        nrf24_getData(data_array);
    }
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

void nrf_enviar(char buff[])
{
    char tamanho=0,i=0,temp;
    tamanho=strlen(buff);
    while(i<=tamanho)
    {
        data_array[0] =buff[i];
        data_array[1] =buff[i+1];
        data_array[2] =buff[i+2];
        data_array[3] =buff[i+3];
        i=i+4;

        nrf24_send(data_array);

        while(nrf24_isSending());

        temp = nrf24_lastMessageStatus();

        if(temp == NRF24_TRANSMISSON_OK)
        {
            enviar("> Tranmission went OK\r\n");
        }
        else if(temp == NRF24_MESSAGE_LOST)
        {
            enviar("> Message is lost ...\r\n");
        }
    }

    temp = nrf24_retransmissionCount();
    sprintf(buffer,"> Retranmission count: %d\r\n",temp);
    enviar(buffer);

    nrf24_powerUpRx();


    _delay_ms(10);
}
void processar_RX()
{
    char buffer_Tx[4];
    char i;
    enviar("Entrei no Rx\r\n");
    switch (rx)
    {
    case '1':
    {
        nrf_enviar("34\r\n");
        do//Espera até que player1 ou player2 enviem '4' (inicio de jogo/disparo)
        {
            enviar("estou a espera de receber14\r\n");
            PORTB|=0x01;
            nrf_receber();

        }
        while(data_array[0]!='1');
        enviar("estou a espera de receber1");
        if((data_array[0]=='1') && (data_array[1]=='4'))
        {
            enviar("estou a espera de receber14");
            PORTB^=0x01;
            sprintf(buffer_Tx,"%c%c%c",data_array[0],data_array[1],'<');
            enviar(buffer_Tx);
        }
        for (i=0; i<strlen(buffer_Tx); i++)
        {
            data_array[i]=0;	//limpa o buffer
        }
        break;
    }
    case '2':
    {
        nrf_enviar("13joaoca\n");
        break;
    }
    }


}


ISR (USART_RX_vect)
{
    rx=UDR0;
    processar_RX();
}

void clear_dataarray()
{
    char i=0;
    for(i=0; i<strlen(data_array); i++)
        data_array[i]='0';
}
/*
char dados_recebidos()
{
    char buff[10];
    nrf_receber();
    if(data_array[0]!='0')
    {
        sprintf(buff,"%c%c%c%c\r\n",data_array[0],data_array[1],data_array[2],data_array[3]);//pode haver problema aqui
        enviar(buff);
          clear_dataarray();
    }

    /*{
    case '1':
    {
        if(data_array[1]=='1')
        {
            sprintf(buff,"municoes %c%c\r\n",data_array[2],data_array[3]);
            enviar(buff);
        }
        else
        {
            if(data_array[1]=='2')
            {
                sprintf(buff,"vida %c%c\r\n",data_array[2],data_array[3]);
                enviar(buff);
            }
        }
        clear_dataarray();
        break;
    }

    case '2':
    {
         if(data_array[1]=='1')
        {
            sprintf(buff,"municoes %c%c\r\n",data_array[2],data_array[3]);
            enviar(buff);
        }
        else
        {
            if(data_array[1]=='2')
            {
                sprintf(buff,"vida %c%c\r\n",data_array[2],data_array[3]);
                enviar(buff);
            }
        }
        clear_dataarray();
        break;
    }
    }
    return '1';
}*/

int main(void)
{
    init();
    nrf24_init();
    nrf24_config(2,4);
    nrf24_tx_address(tx_address);
    nrf24_rx_address(rx_address);
    enviar("Setup concluido");
    clear_dataarray();
    while(1)
    {
        //  dados_recebidos();
    }
}

