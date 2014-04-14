#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "3310.h"
//mudado
#define balas 30

volatile char frase[20];
char nome[]="Joao";
volatile char vida=100,flag=0,cont_20ms=6,cont_sing500ms=10,cont_reload=0,flag_reload;
volatile int pisca=30,municoes=balas;

void setup(void)
{
    DDRB=0b11011111;
    DDRC=0b00011111;
    initlcd();

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
    if(cont_sing500ms==0 && municoes>0)
    {
        cont_sing500ms=10;
        municoes--;

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
    int i=0;
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
    clearram();
    cursorxy(0,0);
    //receber byte do PC
    //if qq
    putstr("Dispare para  comecar");
    cursorxy(0,3);
    putstr("Player 2: ");
    while((EIFR&&0b00000001)!=1);
    municoes=balas;
    clearram();

}
void printmenu()
{
    cursorxy(16,0);
    putstr("LASER TAG");
    cursorxy(0,1);
    putstr("Player: ");
    putstr(nome);
    cursorxy(0,2);
    putstr("Vida: ");
    putint(vida);
    if(vida<100)
        putstr("% ");
    else
        putstr("%");
    cursorxy(0,3);
    if(municoes>0)
    {
        putstr("Municoes: ");
        putstr("  ");
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

//domingo
