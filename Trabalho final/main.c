#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "3310.h"
#include "nrf24.h"
#include <string.h>

#define F_CPU 1000000
#include <util/delay.h>

#define balas 30

#define pedro1 {0xD7,0xD7,0xD7,0xD7,0xD7}  //endereços mac dos modulos
#define pedro2 {0xE7,0xE7,0xE7,0xE7,0xE7}
#define joao1  {0xA7,0xA7,0xA7,0xA7,0xA7}
#define joao2  {0xB7,0xB7,0xB7,0xB7,0xB7}
#define base  {0xC7,0xC7,0xC7,0xC7,0xC7}
//#define player 1

volatile char player='1';

volatile char led_pisca=10,posicao=0,flag_block=0,cont_20ms=6,cont_disparar=-1,cont_disparo=10,cont300ms=6,cont_reload=0,flag_reload,
              pin,cont_vibr1s=0,teste=0,vida=100;

char data_array[4],buffer[30],nome[7];
char vida2=100,ganho=0,perco=0,headshots=0,headshots2=0,pisca=30,flag_disparo=0,flag_head=0,multi=0,flag_single=0;
int municoes=0;
uint8_t rx_address[5] = joao1; //estabelece quais os mac's em uso
uint8_t tx1_address[5] = base; //rx corresponde ao nosso e tx para onde vai enviar
uint8_t tx2_address[5] = pedro1;

void setup(void)
{
    DDRB|=(1<<PB7)|(1<<PB1); //inicialização dos portos
    DDRC|=(1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3)|(1<<PC4)|(1<<PC5);
    DDRD|=(1<<PD6)|(1<<PD7);
    initlcd(); //chama a função para configurar os registos do lcd

    EICRA|=(1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00); //configura o int0 e int1 para gerar a interrupçao no flanco ascendente
    EIMSK|=(1<<INT0)|(1<<INT1); //activa a interrupção externa int0 e int1
    PCICR |=(1<<PCIE2); //ativa os pcint do [16,23]
    PCMSK2 |=(1<<PCINT17)|(1<<PCINT21)|(1<<PCINT16)|(1<<PCINT20); //mascara para ativar apenas os seguintes pcint

    //configuração do timer 1
    TCCR1A = 0;
    TCCR1B |=(1<<WGM12) | (1<<CS10);   // modo CTC, sem prescaler
    OCR1A =  13;     //o tempo de cada instrução é dada por t=1/clock=1 us para o clock de 1MHZ
    //pelo que a interrupção irá demorar 13us para ser executada
    // f=1/13us=76923Hz

    //configuração do timer 2
    // 1 MHz clock com prescaller de 8 demora 8us por instruçao,
    //logo o timer irá incrementar a cada 8us
    //é necessario incrementar 125 vezes para que execute uma interrupcao de 1ms
    TCCR2A|= (1<<WGM21) ;   // CTC mode
    OCR2A  = 124;            // count up to 125  (zero relative!!!!)
    TIMSK2|= (1<<OCIE2A);   // enable Timer2 Interrupt
    TCCR2B|= (1<<CS21);  // // ativa a interrupção do timer 2

    //Configuração do timer 0 para gerar interrupçao de 50ms
    TCCR0A|=(1<<WGM01); //modo ctc
    TCCR0B|=(1<<CS02); //prescaller 256
    OCR0A=194;
    TIMSK0|= 2; //ativa o timer 0

    sei(); //activa as interrupçoes globais

}

/*NOTA: o nosso receptor so detecta o IR caso este tenha um frequência de 38KHz e após alguns testes verificamos
que este tinham uma boa deteçao quando o sinal tinha uma frequencia de 38KHz e tinha o seguinte pulso

tempo:   3ms    3ms   1ms
        ______         ___
              |_______|   |_____

posicao    0     1     2    3      para a alteração do led na posicão 1 basta usar um exor e deixar a mesma
posicao pois o tempo de execução e o mesmo
*/

ISR (TIMER2_COMPA_vect)//interruçao de 1ms
{
    if(cont_disparar>0)  //quando maior que zero decrementa ate que passe o tempo de cada posicao
        cont_disparar--;
    if((posicao==0||posicao==1)&&cont_disparar==0) // o ciclo de dipsaro comeca na posiçao=0
    {
        if(multi==0)
            cont_vibr1s=10;  //variavel para por o motor a vibrar, o tempo é difernete dependo do modo
        else
            cont_vibr1s=15;

        TCCR1A ^=(1<<COM1A0) ;  // toggle OC1A on compare match: o timer 1 ira fazer toggle
        if ((TCCR1A & (1<<COM1A0)) == 0)// ao do led de disparo a cada 13us
            PORTB&=~(1<<PB1);  //  quando na segunda repetica certifica que o led fica apagado
        posicao++;
        cont_disparar=3; //tempo de demora desta posiçao
    }
    if(posicao==2&&cont_disparar==0)
    {
        TCCR1A |=(1<<COM1A0) ; // toggle OC1A on compare match activo novamente
        posicao++;
        cont_disparar=1;
    }
    if(posicao==3&&cont_disparar==0)
    {
        TCCR1A &=~(1<<COM1A0) ;   // toggle OC1A on compare match desativo
        PORTB&=~(1<<PB1);  //desliga o led
        posicao=0;        //volta a por a posição no zero
        cont_disparar=-1;  //com este valor bloqueia o ciclo de disparo ate voltar a existir um disparo
    }
}

void nrf_receber() //funcao para receber pelo nrf
{
    if(nrf24_dataReady()) //se tiver recebido algo, vai buscar os dados e coloca no data array
    {
        nrf24_getData(data_array);
    }
}

void nrf_enviar(char buff[])
{
    char tamanho=0;
    unsigned char i=0;
    uint8_t temp;

    tamanho=strlen(buff);
//envia para a base e para o outro player,


    while(i<=tamanho)
    {
        data_array[0] =buff[i];
        data_array[1] =buff[i+1];
        data_array[2] =buff[i+2];
        data_array[3] =buff[i+3]; //envia os dados num buffer de tamanho 4, e repete ate
        i=i+4;                    //que todos os dados sejam envados

        nrf24_send(data_array);  //envia o array


        while(nrf24_isSending());//espera que o envio seja concluido


        temp = nrf24_lastMessageStatus(); //analisa a transmissao

        if(temp == NRF24_TRANSMISSON_OK)
        {
            //enviar("recebi"); //debug
        }
        else if(temp == NRF24_MESSAGE_LOST)
        {
            //enviar("falhou"); //debug
        }
    }
    /* Retranmission count indicates the tranmission quality */
    temp = nrf24_retransmissionCount();  //indica a qualidade do envio
    //sprintf(buffer,"> Retranmission count: %d\r\n",temp);
    //enviar(buffer);


    nrf24_powerUpRx(); //volta para o modo receptor
    // nrf24_powerDown(); //ou podemos desliga
    _delay_ms(10); //espera um pouco para entrar no modo Rx

}


ISR(TIMER0_COMPA_vect) //interrupcao de 50ms
{
    char municoes_tx[5];

    led_pisca--;
    if(led_pisca==0)  //contador para o led pisca
    {
        PORTB^=(1<<PB7); //altera o estado do led a cada 500ms
        led_pisca=10;
    }

    if(cont300ms>0) //contador para o delay entre os disparos do modo riffle
        cont300ms--;
    if(flag_block==1)//flag para que apenas seja gerada uma interrupçao nos pcint
        cont_20ms--; //contador de delay para que seja activada novamente as interrupçoes pcint
    if((cont_20ms<0)&&(flag_block==0))
        cont_20ms=4;
    if((cont_20ms==0)&&(flag_block==1))
    {
        cont_20ms=6;
        PCICR|=(1<<PCIE2);//activa novamente as interrupçoes pcint
    }
    if(cont_disparo>0) //contador com delay  de disparos no modo single
        cont_disparo--;
    if(flag_reload==1) //precisa de reload
    {
        cont_reload--; //contador de tempo para o reload
        if(cont_reload==0)
        {
            municoes=balas; //coloca novamente o valor maximo das balas
            flag_reload=0; //flag para indicar quando esta a recarrega, quando 0 nao se encontra a carregar
        }
    }
    if(municoes>0 && flag_single==2 && cont300ms==0)  //se este if se verificar é efectuado um disparo no modo riffle
    {
        if((PIND&0b00000100)==0)  //se o botao de disparo deixar de ser primido para de disparar
            flag_single=0;  //este flag quando =1 este  no modo single, quando =0 nao e efectuado nenhum disparo quando =2 sao efectuados
        else               //repetitivamente disparos no modo rifle ate que seja diferente de 2
        {
            municoes--; //sempre que efectuado um disparo e decrementado nas muncioes
            if(municoes>9)
                sprintf(buffer,"%c2%d",player,municoes); //envia o valor das municoes por Rf
            else
                sprintf(buffer,"%c20%d",player,municoes);
            nrf_enviar(buffer);

            cont300ms=6; //carrega o contador de delay entre disparos com o valor 6
            if(flag_reload==0)
            {
                cont_disparar=0; //flag que permite que no timer seja possivel ser executado o ciclo de disparo
            }
        }

    }
    if(cont_vibr1s>0)  //este ciclo tem como objectivo criar a vibração da arma sempre que existe
    {
        //um disparo
        if(cont_vibr1s==10||cont_vibr1s==15)
            PORTC|=(1<<PC5);
        cont_vibr1s--;
        if(multi==0&&cont_vibr1s==0)
            PORTC&=~(1<<PC5);
        if((multi==1&&cont_vibr1s==7))
            PORTC&=~(1<<PC5);
    }
}



ISR(INT0_vect) //disparo PD2 pino4** função de disparo
{
    char municoes_tx1[8];
    if(flag_reload==0) //se estiver a  recarregar
    {
        if(multi==0) //se estiver no modo single
        {
            if(municoes>0&&cont_disparo==0)
            {
                nrf24_tx_address(tx1_address);
                cont_disparo=17; //delay entre cada disparo sera no minimo 17*50ms
                municoes--; //decrementa o numero de municoes
                if(municoes>9)
                    sprintf(municoes_tx1,"%c2%d",player,municoes);//buffer a ser enviado para a base com as municoes
                else
                    sprintf(municoes_tx1,"%c20%d",player,municoes);
                nrf_enviar(municoes_tx1);


                cont_disparar=0;  //flag que permite que no timer seja possivel ser executado o ciclo de disparo
            }

        }
        else
        {
            if(flag_single==0&&cont_disparo==0) //se nao estiver a disparar (flag_single=0), e tiver passado
            {
                //o delay entre o ultimo disparo
                cont_disparo=2;
                flag_single=2;// flag activa os disparos continuos no modo rifle
            }
        }
    }

}

ISR(INT1_vect) //reload PD3 pino 5
{
    flag_reload=1; //activa a flag de recarregar
    cont_reload=40; //o carregamento da arma demora 40*50ms
}

ISR(PCINT2_vect) //rotina de interrupçoes pcint
{

    pin=PIND;
    int health;
    char vida_tx[5],vida_tx1[8];//buffer de envio para a base

    if(((pin&0b00100011)==3)&&flag_block==0)//cabeca PD5// executa se o sensor da cabeça estiver em zero, e o flanco for descendente
    {
        PCICR&=~(1<<PCIE2); // os pcint, que serao activos apos 200ms
        flag_block=1;
        if(vida>0)
        {
            vida=vida-80;
            if(vida<=0)
            {
                vida=0; //decrementa a vida do jogador
                sprintf(vida_tx,"%c109",player);
                sprintf(vida_tx1,"%c1%d9",player,vida);
            } //buffer a ser enviado para a base com a vida do jogador
            else
            {
                sprintf(vida_tx,"%c1%c",player,vida);
                sprintf(vida_tx1,"%c1%d",player,vida);
            }
            nrf24_tx_address(tx1_address);
            nrf_enviar(vida_tx1);
            nrf24_tx_address(tx2_address);
            nrf_enviar(vida_tx);
            if(vida<=0)
                headshots2++; //se a vida for menor ou igual a zero quer dizer que o jogador perdeu com um headshot
        }
    }
    else
    {
        if(((pin&0b00100011)==34)&&flag_block==0)//peito pino2 PD0**semelhante ao da cabeça
        {
            PCICR&=~(1<<PCIE2);
            flag_block=1;
            if(vida>0)
            {
                vida=vida-40;
                if(vida<=0)
                {
                    vida=0; //decrementa a vida do jogador
                    sprintf(vida_tx,"%c100",player);
                    sprintf(vida_tx1,"%c10%d",player,vida);
                } //buffer a ser enviado para a base com a vida do jogador
                else
                {
                    sprintf(vida_tx,"%c1%c",player,vida);
                    sprintf(vida_tx1,"%c1%d",player,vida);
                }
                nrf24_tx_address(tx1_address);
                nrf_enviar(vida_tx1);
                nrf24_tx_address(tx2_address);
                nrf_enviar(vida_tx);
                if(vida<=0)
                    headshots2++; //se a vida for menor ou igual a zero quer dizer que o jogador perdeu com um headshot
            }
        }
        else
        {
            if(((pin&0b00100011)==33)&&flag_block==0)//braço pin3 PD1****semelhante ao da cabeça
            {
                PCICR&=~(1<<PCIE2);
                flag_block=1;
                if(vida>0)
                {
                    vida=vida-20;
                    if(vida<=0)
                    {
                        vida=0; //decrementa a vida do jogador
                        sprintf(vida_tx,"%c100",player);
                        sprintf(vida_tx1,"%c10%d",player,vida);
                    } //buffer a ser enviado para a base com a vida do jogador
                    else
                    {
                        sprintf(vida_tx,"%c1%c",player,vida);
                        sprintf(vida_tx1,"%c1%d",player,vida);
                    }
                    nrf24_tx_address(tx1_address);
                    nrf_enviar(vida_tx1);
                    nrf24_tx_address(tx2_address);
                    nrf_enviar(vida_tx);

                }
            }
            else if(((pin&0b00010000)!=0)&&flag_block==0) //alterar estado disparo pino6 PD4
            {
                PCICR&=~(1<<PCIE2);
                flag_block=1;
                if(multi==1)  //altera o  o modo de disparo
                {
                    multi=0;
                }
                else multi=1;
            }
            else
                flag_block=0;
        }
    }
    if(vida<0)
        vida=0; //se a vida for inferior a zero ela fica com valor zero
}


void printinic()// esta funcão apenas imprime ao inicar o jogo uma mensagem animada a dizer lasertag
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

void clear_data()
{
    unsigned char i=0;
    for(i=0; i<4; i++)
        data_array[i]='a';
}

void inicio()
{
    char i,j=0;


 //incia o modulo RF
    nrf24_init();

 //configura o modulo a funcionar no canal 4, com um buffer de tamanho 4  nrf24_config(2,4);
    nrf24_config(2,4);

  //define o mac do modulos do jogador
    nrf24_tx_address(tx1_address);
    nrf24_rx_address(rx_address);
    clearram();
    cursorxy(0,0);
    putstr("A aguardar pela    base");
    while((data_array[0]!='3')&&(data_array[1]!='4'))//codigo para a base confirmar que pode inicar o jogo
    {
        nrf_receber();
    }
    clearram();
    cursorxy(0,0);
    putstr("Dispare para  comecar");
    while((PIND&0b00000100)==0);//aguarda que seja primido o botao de disparo
    delay_ms(100);
    sprintf(buffer,"%c4\r\n",player);//indica á base que o jogador esta pronto a jogar
    nrf_enviar(buffer);
    clearram();
    cursorxy(0,0);
    putstr("A aguardar pelo dados");
    clear_data();
    do
    {
        nrf_receber();
    }
    while(((data_array[0]!=player)&&(data_array[1]!='3')));//apos a base enviar o numero do player seguido de um3
    //vai ser recebido o nome do jogador
    clearram();
    nome[0]=data_array[2];         //a string recebida e do formato "nºplayer+3+nome\n"
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
    vida=100;
    municoes=balas;
    clearram();
}
void printmenu()
{
    cursorxy(0,0);
    putstr(nome); //imprime o nome no lcd
    cursorxy(36,0);
    if(multi==1)
        putstr("Rifle  ");  //imprime o modo de disparo actual
    else
        putstr("Single");
    cursorxy(0,1);

    putstr("Vida: "); //imprime a vida
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
            putstr("A RECARREGAR     "); //caso esteja a recearregar aparece a mensagem
        }
        else
        {
            if(municoes>=10)
                putstr("Municoes: "); // se nao estiver a recarregar e tiver municoes imprime no lcd
            else                      //quantas municoes tem
                putstr("Municoes:  ");
            putint(municoes);
        }
    }
    else            //a mensagem de a recarregar irá piscar enquanto recarrega
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
    putstr("Adversario: ");  //imprime no lcd a vida do adversario
    putint(vida2);
    if(vida2<100)
        putstr("% ");
    else
        putstr("%");
}



void fim_djogo()
{
    clearram(); //limpa todos os dados do lcd
    cursorxy(16,3);
    if(vida2<=0)
    {
        putstr("YOU WIN!!");
        PORTD|= (1<<PD6);//indica que ganhamos ou perdemos
        PORTD&=~(1<<PD7);
    }
    else
    {
        putstr("Game Over");
        PORTD|= (1<<PD7);//indica que ganhamos ou perdemos
        PORTD&=~(1<<PD6);
    }
    if(flag_head==1)  //caso tenhamos efectuado um headshot indica a mensagem
    {
        cursorxy(8,4);
        putstr("@ HEADSHOT!!!");
        flag_head=0;
    }
    delay_ms(3000);
    clearram();// limpa o ecra e mostra uma grelha com os resultados das jogadas totais
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
    PORTD&=~(1<<PD6);
    PORTD&=~(1<<PD7);
    putstr("Dispara para  recomecar...");
    while((PIND&0b00000100)==0); //recomeça o jogo
    delay_ms(100);
    vida2=100;
    municoes=balas;
    vida=100;
    clearram();
    printmenu();
}



int main(void)
{

    setup(); //chama a funcção para configurar registo e timers
    printinic(); //funçao de incializaçao do ecra de entrada do lcd. frase: laser tag...
    inicio();//função onde é inciado o processo do jogo

    while(1)
    {
        if(vida>0)
            printmenu(); // chama a função enquanto a vida do jogador for maior que zero
        else
        {
            perco++;   //se a vida for menor ou igual a zero incrementa o numero de derrotas
            fim_djogo(); //e chama a função fim de jogo
        }
        clear_data();  //limpa o data array
        nrf_receber(); //tenta receber a vida do adversario
        if(data_array[0]!='a')  //verifica se recebeu algo
        {
            if(data_array[1]=='1')
            {
                vida2=data_array[2]; //caso a vida seja igual a zero indica que ganhamos
                if(vida2<=0)
                {
                    ganho++;
                    if(data_array[3]=='9') //se seguido da vida vier 9 indica que ganhamos por headshot
                    {
                        flag_head=1; //activa a flag de hedshot para ser diferenciado na função fimd de jogo
                        headshots++; //incrementa o numero de headshots efectuados
                    }
                    fim_djogo();  //indica que o jogo terminou
                }
            }
        }
    }
    return 0;
}
