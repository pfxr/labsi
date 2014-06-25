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
#define base  {0xC7,0xC7,0xC7,0xC7,0xC7}

#define FOSC 1000000
#define F_CPU 1000000
#define BAUD 4800
#define USART_UBBR_VALUE FOSC/16/BAUD-1

//char buffer_Tx[200];
volatile unsigned char rx;

char data_array[4],cont_players=0,buffer[40],buffer_rx[15];

volatile char cont_rx=0,StrRxFlag=0,prontos=0,comecou=0;

uint8_t rx_address[5] = base;
uint8_t tx1_address[5] = joao1;
uint8_t tx2_address[5] = pedro1;

void init (void)
{
    DDRC|=(1<<PC5)|(1<<PC4);
    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8);
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C= 0b00000110;
    UCSR0B= 0b00011000;
    UCSR0B|=0x80;

    SREG |= 0x80;			//Autorizacao global das interrupcoes

}
void nrf_receber()
{
    PORTC|=(1<<PC4);
    if(nrf24_dataReady())
    {
        nrf24_getData(data_array);
    }
    PORTC&=~(1<<PC4);
}




void enviar(char *Tx)
{
    int i=0;
    PORTC|=(1<<PC5);
    while(i<strlen(Tx))
    {
        while(!(UCSR0A & (1<<UDRE0)));
        UDR0=Tx[i];
        i++;
        PORTC&=~(1<<PC5);
    }



}

void nrf_enviar(char buff[])
{

    char tamanho=0,i=0,temp;
    PORTC|=(1<<PC4);
    tamanho=strlen(buff);
    while(i<tamanho)
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
            //enviar("> Tranmission went OK\r\n");
        }
        else if(temp == NRF24_MESSAGE_LOST)
        {
            // enviar("> Message is lost ...\r\n");
        }
    }

    temp = nrf24_retransmissionCount();
    sprintf(buffer,"> Retranmission count: %d\r\n",temp);
    //  enviar(buffer);

    nrf24_powerUpRx();


    _delay_ms(10);
    PORTC&=~(1<<PC4);
    // enviar("nrf a enviar\r\n");
}

void clear_dataarray()
{
    char i=0;
    for(i=0; i<4; i++)
        data_array[i]='0';
}

void processar_RX(char rx[])
{
    char buffer_Tx[10],buff[10];
    char i;
    // enviar("Entrei no Rx\r\n");
    switch (rx[0])
    {
    case '1':
    {
        //enviar("entrei no case1");
        nrf24_tx_address(tx1_address);
        nrf_enviar("34");
        nrf24_tx_address(tx2_address);
        nrf_enviar("34");
        while(comecou<2)
        {    clear_dataarray();
            do//Espera até que player1 ou player2 enviem '4' (inicio de jogo/disparo)
            {
                //    enviar("estou a espera de receber14\r\n");
                nrf_receber();

            }
            while(data_array[0]=='0');

            if((data_array[0]=='1') && (data_array[1]=='4'))
            {
                //  enviar("estou a espera de receber14");
                sprintf(buffer_Tx,"%c%c\r\n",data_array[0],data_array[1]);
                enviar(buffer_Tx);
                comecou++;
            }

                 if((data_array[0]=='2') && (data_array[1]=='4'))
            {
                //  enviar("estou a espera de receber14");
                sprintf(buffer_Tx,"%c%c\r\n",data_array[0],data_array[1]);
                enviar(buffer_Tx);
                comecou++;
            }


        }
        break;
    }
    case '2':
    {
        if(rx[1]=='1')
            nrf24_tx_address(tx1_address);
        else
            nrf24_tx_address(tx2_address);
        sprintf(buff,"%c3%s",rx[1],(rx+2));
        nrf_enviar(buff);
        //enviar(buff);
        prontos++;
        break;
    }
    }
}


ISR (USART_RX_vect)
{
    PORTC|=(1<<PC5);
    buffer_rx[cont_rx]=UDR0;         //Read USART data register
    if(buffer_rx[cont_rx]=='\n')   //check for carriage return terminator and increment buffer index
    {
        // if terminator detected
        //Set String received flag
        //buffer_rx[cont_rx-1]=0x00;   //Set string terminator to 0x00
        cont_rx=0;                //Reset buffer index
        //  enviar(buffer_rx);
        processar_RX(buffer_rx);//enviar(buffer_rx);
        buffer_rx[0]='\0';

    }
    else cont_rx++;
    PORTC&=~(1<<PC5);
}


void dados_recebidos()
{
    char buff[7];
    clear_dataarray();
    do
    {
        nrf_receber();
    }
    while(data_array[0]=='0');

    sprintf(buff,"%c%c%c%c\r\n",data_array[0],data_array[1],data_array[2],data_array[3]);
    enviar(buff);
}

int main(void)
{
    init();
    nrf24_init();
    nrf24_config(2,4);
    buffer_rx[0]=0;
    nrf24_tx_address(tx1_address);
    nrf24_rx_address(rx_address);
    enviar("Setup concluido");
    clear_dataarray();
    while(1)
    {
        if(prontos==2)
            dados_recebidos();
    }
}

