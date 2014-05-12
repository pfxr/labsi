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

char data_array[4],cont_players=0,buffer[40],buffer_rx[15];

volatile char cont_rx=0,StrRxFlag=0;

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
     enviar("nrf a enviar\r\n");
}
void processar_RX(char rx[])
{
    char buffer_Tx[4],buff[10];
    char i;
    // enviar("Entrei no Rx\r\n");
    switch (rx[0])
    {
    case '1':
    {   //enviar("entrei no case1");
        nrf_enviar("34\r\n");
        do//Espera até que player1 ou player2 enviem '4' (inicio de jogo/disparo)
        {
            //    enviar("estou a espera de receber14\r\n");
            nrf_receber();

        }
        while(data_array[0]!='1');
        //  enviar("estou a espera de receber1");
        if((data_array[0]=='1') && (data_array[1]=='4'))
        {
            //  enviar("estou a espera de receber14");
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
        PORTB=0x01;
        sprintf(buff,"13%s",(rx+1));
        nrf_enviar(buff);
        // enviar(buff);
        break;
    }
    }


}


ISR (USART_RX_vect)
{
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

}

void clear_dataarray()
{
    char i=0;
    for(i=0; i<4; i++)
        data_array[i]='0';
}

void dados_recebidos()
{
    char buff[10];
    clear_dataarray();
    while(data_array[0]=='0')
    {
        nrf_receber();
    }
    switch(data_array[0])
    {
    case '1':
    {
        if(data_array[1]=='1')
        {
            sprintf(buff,"%c%c%c%c%c",data_array[0],data_array[1],data_array[2],data_array[3],'<');
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

}

int main(void)
{
    init();
    nrf24_init();
    nrf24_config(2,4);
    buffer_rx[0]=0;
    nrf24_tx_address(tx_address);
    nrf24_rx_address(rx_address);
    enviar("Setup concluido");
    clear_dataarray();
    while(1)
    {
        dados_recebidos();
    }
}

