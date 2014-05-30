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

volatile char vida=100,flag=0,cont_20ms=6,cont_sing500ms=10,cont300ms=6,cont_reload=0,flag_reload;
volatile int tim=10;
volatile char pisca=30,flag_disparo=0,municoes,flag_head=0,multi=0,flag_single=0,pin,cont_vibr1s=0,teste=0;
char data_array[4],buffer[30],nome[15]="Pedro";
char vida2=100,ganho=0,perco=0,headshots=0,headshots2=0;
volatile int x;
uint8_t temp;
uint8_t rx_address[5] = joao1;
uint8_t tx_address[5] = pedro1;

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
    //uart_init();
    EICRA=0b00001101;
    EIMSK=0x03;
    PCMSK2 |=(1<<PCINT17);
    PCMSK2 |=(1<<PCINT21);
    PCMSK2 |=(1<<PCINT16);
    PCMSK2 |=(1<<PCINT20);
    PCICR |=(1<<PCIE2);

    TCCR1A=0b00000000;
    TCCR1B=0b00001001;
    OCR1A=0x999;
    TIMSK1|= 2;


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
            PORTB^=(1<<PB0);
            delay_ms(100);
            PORTB^=(1<<PB0);

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



ISR(TIMER1_COMPA_vect)
{
    if(tim>0)
        tim=tim-1;

    /*if(teste==0)
        teste=250;
    teste--;
    if(teste==0)
        PORTB^=(1<<PB7);*/
}


void disparo()
{
    int i=0,j=0;
    if(multi==0)
        cont_vibr1s=10;
    else
        cont_vibr1s=15;
    if(tim==0)
    {
        tim=3;
        while(tim>0)
        {
            PORTB|=(1<<PB7);
            _delay_us(13);
            PORTB&=~(1<<PB7);
            _delay_us(13);
        }
        tim=3;
        while(tim!=0);
        tim=1;
        while(tim>0)
        {
            PORTB|=(1<<PB7);
            _delay_us(13);
            PORTB&=~(1<<PB7);
            _delay_us(13);
        }
        tim=5;
    }
}

ISR(TIMER0_COMPA_vect) //tempos
{
    if(cont300ms>0)
        cont300ms--;
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
    if(flag_reload==1) //precisa de reload
    {
        cont_reload--;
        if(cont_reload==0)
        {
            municoes=balas;
            flag_reload=0;
        }
    }
    if(municoes>0)
    {
        if(flag_single==2 && cont300ms==0)
        {
            if((PIND&0b00000100)==0)
                flag_single=0;
            municoes--;
            cont300ms=6;
            //disparo();
            flag_disparo=1;
        }
    }
    if(cont_vibr1s>0)
    {
        if(cont_vibr1s==10||cont_vibr1s==15)
            PORTB|=(1<<PB6);
        cont_vibr1s--;
        if(multi==0&&cont_vibr1s==0)
            PORTB&=~(1<<PB6);
        if((multi==1&&cont_vibr1s==7))
            PORTB&=~(1<<PB6);
    }
}



ISR(INT0_vect) //disparo PD2 pino4
{
    if(multi==0)
    {
        if(municoes>0)
        {
            if((flag_single==0)&&(cont_sing500ms==0))
            {
                cont_sing500ms=10;
                municoes--;
                flag_single=1;
                flag_disparo=1;
                // disparo();
            }
            else
            {
                if(flag_single!=0)
                {
                    cont_sing500ms=10;
                    flag_single=0;
                }
            }
        }
    }
    else
    {
        if(flag_single==0&&cont_sing500ms==0)
        {
            cont_sing500ms=2;
            flag_single=2;
        }
        else
        {
            if(flag_single!=0&&cont_sing500ms==0)
            {
                cont_sing500ms=10;
                flag_single=0;
            }
        }

    }

}

ISR(INT1_vect) //reload PD3 pino 5
{
    flag_reload=1;
    cont_reload=40;
}

ISR(PCINT2_vect) // vida
{

    pin=PIND;
    char vida_tx[5];

    if(((pin&0b00110011)==3)&&flag==0)//cabeça pino11 PD5
    {
        PCICR&=~(1<<PCIE2);
        flag=1;
        if(vida>0)
        {
            vida=vida-80;
            sprintf(vida_tx,"%c1",vida);
            nrf_enviar(vida_tx);
            if(vida<=0)
                headshots2++;
            PORTB^=(1<<PB6);
        }
    }
    else
    {
        if(((pin&0b00110011)==34)&&flag==0)//peito pino2 PD0
        {
            PCICR&=~(1<<PCIE2);
            flag=1;
            if(vida>0)
            {
                vida=vida-40;
                sprintf(vida_tx,"%c",vida);
                nrf_enviar(vida_tx);
                PORTB^=(1<<PB6);
            }
        }
        else
        {
            if(((pin&0b00110011)==33)&&flag==0)//braço pin3 PD1
            {
                PCICR&=~(1<<PCIE2);
                flag=1;
                if(vida>0)
                {
                    vida=vida-20;
                    sprintf(vida_tx,"%c",vida);
                    nrf_enviar(vida_tx);
                    PORTB^=(1<<PB6);
                }
            }
            else if(((pin&0b00010000)!=0)&&flag==0) //alterar estado disparo pino6 PD4
            {
                PCICR&=~(1<<PCIE2);
                flag=1;
                if(multi==1)
                {
                    multi=0;
                }
                else multi=1;
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
    // char i,j=0;

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
    // putstr("A espera 34");
    // while((data_array[0]!='3')&&(data_array[1]!='4'))//codigo para a base confirmar que pode inicar o jogo
    //{
    //   nrf_receber();
    //  }
    clearram();
    cursorxy(0,0);
    putstr("Dispare para  comecar");
    while((EIFR&0b00000001)!=1);
    delay_ms(100);
    // sprintf(buffer,"%d4\r\n",player);
    //  nrf_enviar(buffer);
    clearram();
    cursorxy(0,0);
    //  putstr("enviei14 espero receber1");
    //  do
    //  {
    //      nrf_receber();
    // }
    // while((data_array[0]!=player)&&(data_array[1]!='3'));
    //  putstr("passei");
    // clearram();
    // nome[0]=data_array[2];
    // nome[1]=data_array[3];
    /* i=2;
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
    */ enviar(nome);
    vida=100;
    municoes=balas;
    clearram();

}
void printmenu()
{
    cursorxy(0,0);
    putstr(nome);
    cursorxy(36,0);
    if(multi==1)
        putstr("Rifle  ");
    else
        putstr("Single");
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
    if(flag_reload==1)
        {
            putstr("A RECARREGAR     ");

        }else
        {
        if(municoes>=10)
            putstr("Municoes: ");
        else
            putstr("Municoes:  ");
        putint(municoes);
        }
    }
    else
    {
        if(flag_reload==1)
        {
            putstr("A RECARREGAR     ");

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
    cursorxy(0,4);
    putstr("Joao: ");
    putint(vida2);
    if(vida2<100)
        putstr("% ");
    else
        putstr("%");
}



void gameover()
{
    //gameover
    clearram();
    cursorxy(16,3);
    if(vida2<=0)
        putstr("YOU WIN!!");
    else
        putstr("Game Over");
    if(flag_head==1)
    {
        cursorxy(8,4);
        putstr("@ HEADSHOT!!!");
        flag_head=0;
    }
    delay_ms(3000);
    clearram();
    cursorxy(6,0);
    putstr("V | D | @ ");
    cursorxy(0,1);
    putstr("P__|___|__");
    cursorxy(6,2);
    putint(ganho);
    putstr(" | ");
    putint(perco);
    putstr(" | ");
    putint(headshots);
    cursorxy(0,3);
    putstr("J__|___|__");
    cursorxy(6,4);
    putint(perco);
    putstr(" | ");
    putint(ganho);
    putstr(" | ");
    putint(headshots2);
    delay_ms(10000);
    clearram();
    cursorxy(0,4);
    putstr("Dispara para  recomecar...");
    while((EIFR&0b00000001)!=1);
    delay_ms(100);
    vida2=100;
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
        //PORTB|=(1<<PB0);
        sprintf(buffer_Tx,"a");
        enviar(buffer_Tx);
    }
    else
        // PORTB&=~(1<<PB0);
        flag_rx=0;
}

void clear_data()
{
    char i=0;
    for(i=0; i<4; i++)
        data_array[i]='a';
}

int main(void)
{

    setup();
    printinic();
    inicio();

    while(1)
    {
        if(flag_disparo==1)//criei este if, pois chamar a funçao disparo no timer ou
        {
            //dentro da funcao de int0 dava problemas
            disparo();
            flag_disparo=0;
        }
        if(vida>0)
            printmenu();
        else
        {
            perco++;
            gameover();
        }
        clear_data();
        nrf_receber();
        if(data_array[0]!='a')
        {
            vida2=data_array[0];
            if(vida2<=0)
            {
                ganho++;
                if(data_array[1]=='1')
                {
                    flag_head=1;
                    headshots++;
                }
                gameover();
            }
        }
    }
    return 0;
}


