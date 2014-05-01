#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "3310.h"
#include "nrf24.h"
#include <string.h>
#include <util/delay.h>

#define balas 30

#define pedro1 {0xD7,0xD7,0xD7,0xD7,0xD7}
#define pedro2 {0xE7,0xE7,0xE7,0xE7,0xE7}
#define joao1  {0xA7,0xA7,0xA7,0xA7,0xA7}
#define joao2  {0xB7,0xB7,0xB7,0xB7,0xB7}
#define player 1

#define FOSC 1000000
#define F_CPU 1000000
#define BAUD 2400
#define USART_UBBR_VALUE FOSC/16/BAUD-1


volatile unsigned char rx,flag_rx;

volatile char vida=100,flag=0,cont_20ms=6,cont_sing500ms=10,cont_reload=0,flag_reload;
volatile char pisca=30,municoes;
char data_array[4],buffer[30],nome[15]="";;

uint8_t temp;
uint8_t tx_address[5] = joao1;
uint8_t rx_address[5] = pedro1;

void uart_init (void)
{
    UBRR0H = (unsigned char)(USART_UBBR_VALUE>>8);
    UBRR0L = (unsigned char) (USART_UBBR_VALUE);

    UCSR0C= 0b00000110;
    UCSR0B= 0b00011000;
    UCSR0B|=0x80;
}

void setup(void)
{
    DDRB=0b11011111;
    DDRC|=0b00011111;
    initlcd();
    uart_init();
    EICRA=0b00001111;
    EIMSK=0x03;
    PCICR |=(1<<PCIE2);
    PCMSK2 |=(1<<PCINT17);
    PCMSK2 |=(1<<PCINT21);
    PCMSK2 |=(1<<PCINT16);
    TCCR0A=0b00000010;
    TCCR0B=0b00000100;
    OCR0A=194;

    TIMSK0|= 2;
    SREG |= 0x80;
}


void enviar(char *Tx)
{
    char i=0;

    while(i<strlen(Tx))
    {
        while(!(UCSR0A & (1<<UDRE0)));
        UDR0=Tx[i];
        i++;
    }
}

void nrf_receber()
{
    if(nrf24_dataReady())
    {
        nrf24_getData(data_array);
        enviar(data_array);
    }
}

void nrf_enviar(char buff[])
{
    char tamanho=0,i=0;

    tamanho=strlen(buff);
    while(i<=tamanho)
    {
        data_array[0] =buff[i];
        data_array[1] =buff[i+1];
        data_array[2] =buff[i+2];
        data_array[3] =buff[i+3];
        i=i+4;
        /* Automatically goes to TX mode */
        nrf24_send(data_array);

        /* Wait for transmission to end */
        while(nrf24_isSending());

        /* Make analysis on last tranmission attempt */
        temp = nrf24_lastMessageStatus();

        if(temp == NRF24_TRANSMISSON_OK)
        {
            enviar("Tranmission went OK\r\nenviei");
            enviar(buff);
        }
        else if(temp == NRF24_MESSAGE_LOST)
        {
            enviar("> Message is lost ...\r\n");
        }
    }
    /* Retranmission count indicates the tranmission quality */
    temp = nrf24_retransmissionCount();
    sprintf(buffer,"> Retranmission count: %d\r\n",temp);
    enviar(buffer);

    /* Optionally, go back to RX mode ... */
    nrf24_powerUpRx();

    /* Or you might want to power down after TX */
    // nrf24_powerDown();

    /* Wait a little ... */
    _delay_ms(10);
}

ISR(TIMER0_COMPA_vect) //tempos
{
    if(flag==1)
        cont_20ms--;
    if((cont_20ms<0)&&(flag==0))
        cont_20ms=4;
    if((cont_20ms==0)&&(flag==1))
    {
        cont_20ms=6;
        PCICR|=(1<<PCIE2);
    }
    if(cont_sing500ms>0)
        cont_sing500ms--;
    if(cont_sing500ms>2)
        PORTB^=(1<<PB7);
    if(cont_sing500ms==2)
        PORTB&=~(1<<PB7);
    if(flag_reload==1) //precisa de reload
    {
        PORTB|=(1<<PB0);
        cont_reload--;
        if(cont_reload==0)
        {
            PORTB&=~(1<<PB0);
            municoes=balas;
            flag_reload=0;
        }
    }
}

ISR(INT0_vect) //disparo PD2 pino4
{
    char buff[10];
    if(cont_sing500ms==0 && municoes>0)
    {
        cont_sing500ms=10;
        municoes--;
        sprintf(buff,"%d1%d",player,municoes);
        nrf_enviar(buff);

        PORTB|=(1<<PB7); //pino 10
    }
}

ISR(INT1_vect) //reload PD3 pino 5
{
    flag_reload=1;
    cont_reload=40;
}

ISR(PCINT2_vect) // vida
{
    char pin=PIND;
    if(((pin&0b00100000)!=0)&&flag==0)//cabeça pino11 PD5
    {
        PCICR&=~(1<<PCIE2);
        flag=1;
        if(vida>0)
        {
            vida=vida-80;
            PORTB^=(1<<PB6);
        }
    }
    else
    {
        if(((pin&0b00000001)!=0)&&flag==0)//peito pino2 PD0
        {
            PCICR&=~(1<<PCIE2);
            flag=1;
            if(vida>0)
            {
                vida=vida-40;
                PORTB^=(1<<PB6);
            }
        }
        else
        {
            if(((pin&0b00000010)!=0)&&flag==0)//braço pin3 PD1
            {
                PCICR&=~(1<<PCIE2);
                flag=1;
                if(vida>0)
                {
                    vida=vida-20;
                    PORTB^=(1<<PB6);
                }
            }
            else
                flag=0;
        }
    }
    if(vida<0)
        vida=0;
}


void printinic()
{
    char i=0;
    cursorxy(16,0);
    putstr("LASER TAG");
    cursorxy(4,2);
    putstr("A iniciar");
    for(i=0; i<6; i++)
    {
        delay_ms(100);
        putstr(".");
    }
    clearram();
}
void inicio()
{
    char i,j=0;

    enviar("\r\n> TX device ready\r\n");

    /* init hardware pins */
    nrf24_init();

    /* Channel #2 , payload length: 4 */
    nrf24_config(2,4);

    /* Set the device addresses */
    nrf24_tx_address(tx_address);
    nrf24_rx_address(rx_address);
    clearram();
    cursorxy(0,0);
    putstr("A espera 34");
    while((data_array[0]!='3')&&(data_array[1]!='4'))//codigo para a base confirmar que pode inicar o jogo
    {
        nrf_receber();
    }
    clearram();
    cursorxy(0,0);
    putstr("Dispare para  comecar");
    while((EIFR&&0b00000001)!=1);
    sprintf(buffer,"%d4\r\n",player);
    nrf_enviar(buffer);
    clearram();
    cursorxy(0,0);
    putstr("enviei14 espero receber1");
    do
    {
        nrf_receber();
    }
    while((data_array[0]!=player)&&(data_array[1]!='3'));
    putstr("passei");
    clearram();
    nome[0]=data_array[2];
    nome[1]=data_array[3];
    i=2;
    while(data_array[j]!='\n')
    {
        if(nrf24_dataReady())
        {
            nrf24_getData(data_array);
            for(j=0; j<=3; j++)
            {
                if(data_array[j]=='\n')
                    break;
                nome[i]=data_array[j];

                i++;
            }
        }
    }
    enviar(nome);
    vida=100;
    municoes=balas;
    clearram();

}
void printmenu()
{
    cursorxy(0,0);
    putstr("Player: ");
    putstr(nome);
    cursorxy(0,1);
    putstr("Vida: ");
    putint(vida);
    if(vida<100)
        putstr("% ");
    else
        putstr("%");
    cursorxy(0,2);
    if(municoes>0)
    {
        putstr("Municoes: ");
        cursorxy(55,3);

        putint(municoes);
    }
    else
    {
        if(pisca>15)
        {
            putstr("RECARREGAR       ");
            pisca--;
        }
        else
        {
            pisca--;
            if(pisca==0)
                pisca=30;
            putstr("                 ");
        }
    }
}



void gameover()
{
    clearram();
    cursorxy(16,0);
    putstr("GAME OVER");
    cursorxy(0,3);
    putstr("Dispara para  recomecar...");
    delay_ms(3000);
    municoes=balas;
    vida=100;
    clearram();
    printmenu();

}

ISR (USART_RX_vect)
{
    rx=UDR0;
    flag_rx=1;
}

void processar_RX()
{
    char buffer_Tx[40];

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

int main(void)
{
    setup();
    printinic();
    inicio();
    while(1)
    {
        if(vida>0)printmenu();
        else gameover();
    }
    return 0;
}


