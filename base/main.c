#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../nrf24.h"

#define pedro1 {0xD7,0xD7,0xD7,0xD7,0xD7}  //define dos mac de cada modulo RF
#define pedro2 {0xE7,0xE7,0xE7,0xE7,0xE7}
#define joao1  {0xA7,0xA7,0xA7,0xA7,0xA7}
#define joao2  {0xB7,0xB7,0xB7,0xB7,0xB7}
#define base   {0xC8,0xC8,0xC8,0xC8,0xC8}

#define FOSC 1000000
#define F_CPU 1000000
#define BAUD 4800
#define USART_UBBR_VALUE FOSC/16/BAUD-1 //calculo para o valor de ubrr da usart

//char buffer_Tx[200];
volatile unsigned char rx,prontos=0,comecou=0;


char data_array[4],cont_players=0,buffer[40],buffer_rx[15];

volatile char StrRxFlag=0,cont=0,cont1=10;
unsigned char cont_rx=0;

uint8_t rx_address[5] = base;
uint8_t tx1_address[5] = pedro1;
uint8_t tx2_address[5] = pedro2;

void init (void)
{
    DDRC|=(1<<PC5);
    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8); //insere o valor calculado previamente
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C|=(1<<UCSZ01)|(1<<UCSZ00); //usart configurada com 8bit de dado e 1 stopbit
    UCSR0B|=(1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); //activa a interruopçao de recepçao completa e a transmissao e recepcao activas

    sei();			//Autorizacao global das interrupcoes

}


void nrf_receber() //funçao igual a existente no codigo da pistola
{
    if(nrf24_dataReady())
    {
        nrf24_getData(data_array);
    }
}




void enviar(char *Tx) //funcao para enviar pela usar para o pc
{
    unsigned char i=0;
    PORTC|=(1<<PC5);
    while(i<strlen(Tx))
    {
        while(!(UCSR0A & (1<<UDRE0)));
        UDR0=Tx[i]; //envia caracter a caracter pela usart até que i atinja o tamnho da string-1
        i++;
    }
    PORTC&=~(1<<PC5);
}

void nrf_enviar(char buff[]) //função igual ao codigo da pistola
{
    unsigned char i=0;
    char tamanho=0,temp;
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
}

void clear_dataarray() //funcao para limpar o data array
{
    unsigned char i=0;
    for(i=0; i<4; i++)
        data_array[i]='0';
}

void processar_RX(char rx[])  //função de incio de jogo, processa os comandos vindos pela usart
{
    char buffer_Tx[10],buff[10];
    unsigned char i;

    switch (rx[0])
    {
    case '1':
    {
        nrf24_tx_address(tx1_address);//envia para os 2 jogadores 34 a indicar que a base esta pronta
        nrf_enviar("34\r\n");
        nrf24_tx_address(tx2_address);
        nrf_enviar("34\r\n");
        do
        {
            clear_dataarray();
            do//Espera até que player1 ou player2 enviem '4' (inicio de jogo/disparo)
            {
                nrf_receber();
            }
            while(data_array[0]=='0');
            //  enviar("estou a espera de receber1");
            if((data_array[0]=='1') && (data_array[1]=='4'))
            {
                enviar("14\r\n"); //envia para a aplicao a indicar que o jogador 1 esta pronto
                prontos++;
            }
            else
            {
                if((data_array[0]=='2') && (data_array[1]=='4'))
                {
                    enviar("24\r\n");//envia para a aplicao a indicar que o jogador 2 esta pronto
                    prontos++;
                }
            }

            for (i=0; i<strlen(buffer_Tx); i++)
            {
                data_array[i]=0;	//limpa o buffer
            }
        }
        while(prontos!=2);
        break;
    }
    case '2':
    {
        if(rx[1]=='1')
        {
            nrf24_tx_address(tx1_address);
            PORTB=0x01;
            sprintf(buff,"13%s",(rx+2)); //a aplicação envia para o player o nome, o player e
            nrf_enviar(buff);           // o player e defenido pelo rx[1]
            break;
        }
        else
        {
            if(rx[1]=='2')
            {
                nrf24_tx_address(tx2_address);
                PORTB=0x01;                 //analoo ao if de cima mas agora e o outro player
                sprintf(buff,"23%s",(rx+2));
                nrf_enviar(buff);
                break;
            }
        }
        comecou=1;
    }
    }


}


ISR (USART_RX_vect)
{
    PORTC|=(1<<PC5);  //indica no led que esta a receber dados da usar
    buffer_rx[cont_rx]=UDR0;        //coloca o caracter no vector buffer
    if(buffer_rx[cont_rx]=='\n')   //a leitura da string recebida termina quando receber o caracter \n
    {
        cont_rx=0;                //lipa o index do vector
        //  enviar(buffer_rx);
        PORTC&=~(1<<PC5);  //indica que terminou de receber dados da suart
        processar_RX(buffer_rx); //procesa os dados recebidos chamando a funcao
        buffer_rx[0]='\0';    //limpar ao buffer

    }
    else cont_rx++; //incrementa o indice do vector

}



void dados_recebidos()
{
    char buff[5];

    clear_dataarray();
    do
    {
        nrf_receber();
    }
    while(data_array[0]=='0');
    sprintf(buff,"%s%c",data_array,'<');//pega no data array e coloca o caracter < para  app ler ate esse ponto
    enviar(data_array); //envia para a app pela usart

}

int main(void)
{
    init();
    nrf24_init(); //inicia o modlo nrf
    nrf24_config(2,4); //configura para o chanel 2 e o tamanho do buffer 4
    buffer_rx[0]=0;
    nrf24_rx_address(rx_address); //define o mac rx do modulod
    enviar("Setup concluido\r\n");
    clear_dataarray();
    while(1)
    {
        if(comecou==1)// se o jogo tiver comecado chama a funcao
            dados_recebidos();
    }
}
