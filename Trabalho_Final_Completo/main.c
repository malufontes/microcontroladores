#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "grlib/grlib.h"

#include "Nokia5110.h"
#include "bitmaps.h"

#include <stdlib.h>
#include "driverlib/timer.h"
#include <math.h>

#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "inc/hw_gpio.h"

#include <time.h>



#define R GPIO_PIN_1
#define B GPIO_PIN_2
#define G GPIO_PIN_3
#define SW_1 GPIO_PIN_4
#define SW_2 GPIO_PIN_0

uint32_t frequencia_timer0;

char *tela[] = {"tela_inicial", "MENU", "instrucoes", "jogo", "GAME OVER"};
uint32_t controle_tela = 0;

uint32_t controle_menu = 0;

//******* || JOGO || ***********
uint32_t ret1[] = {0,0};
uint32_t ret2[] = {0,100};
uint32_t ret3[] = {0,100};
uint32_t ret4[] = {0,100};

uint32_t x_cursor = 28;

uint32_t pontuacao = 0;
//******************************

void display_tela_inicial();

void display_menu();

void display_jogo();



void display_game_over(void){

    Nokia5110_Clear();

    Nokia5110_SetCursor(1, 1);
    Nokia5110_OutString(tela[controle_tela]);

    Nokia5110_SetCursor(1, 3);
    Nokia5110_OutString("Pontuacao:");

    Nokia5110_SetCursor(1, 4);
    Nokia5110_OutUDec(pontuacao);

    SysCtlDelay(SysCtlClockGet() / 3);

}

void SW_1_game_over(void){
    pontuacao = 0;
    ret1[0]=0;
    ret1[1]=0;
    ret2[0]=0;
    ret2[1]=100;
    ret3[0]=0;
    ret3[1]=100;
    ret4[0]=0;
    ret4[1]=100;
    x_cursor=28;

    controle_tela = 0;
    display_tela_inicial();
}

void SW_2_game_over(void){
    pontuacao = 0;
    ret1[0]=0;
    ret1[1]=0;
    ret2[0]=0;
    ret2[1]=100;
    ret3[0]=0;
    ret3[1]=100;
    ret4[0]=0;
    ret4[1]=100;
    x_cursor=28;

    controle_tela = 0;
    display_tela_inicial();
}



void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    if(GPIOPinRead(GPIO_PORTF_BASE, G )){
        GPIOPinWrite(GPIO_PORTF_BASE, G , 0x00 );
    }else{
        GPIOPinWrite(GPIO_PORTF_BASE, G , 0xFF );
    }

    display_jogo();

}

bool game_over(void){

    if(ret1[1]==36){
        if(x_cursor!=ret1[0]){
            return true;
        }else{
            pontuacao++;
        }
    }

    if(ret2[1]==36){
        if(x_cursor!=ret2[0]){
            return true;
        }else{
            pontuacao++;
        }
    }

    if(ret2[1]==36){
        if(x_cursor!=ret2[0]){
            return true;
        }else{
            pontuacao++;
        }
    }

    if(ret3[1]==36){
        if(x_cursor!=ret3[0]){
            return true;
        }else{
            pontuacao++;
        }
    }

    return false;

}

uint32_t gerar_num_rand(){
    uint32_t num = (28 * (rand()%3));
    return num;
}

void display_jogo(){
    Nokia5110_Clear();
    Nokia5110_ClearBuffer();


    if(ret1[1]==0){
        ret1[0] = gerar_num_rand();
    }
    Nokia5110_PrintBMP2(ret1[0], ret1[1] ,retpreto,28,12);


    if(ret2[1]==0){
        ret2[0] = gerar_num_rand();
    }
    if(ret1[1]==12&&ret2[1]==100){
        ret2[0] = gerar_num_rand();
        ret2[1] = 0;
    }
    if(ret2[1]!=100){
        Nokia5110_PrintBMP2(ret2[0], ret2[1] ,retpreto,28,12);
    }


    if(ret3[1]==0){
        ret3[0] = gerar_num_rand();
    }
    if(ret1[1]==24&&ret3[1]==100){
        ret3[0] = gerar_num_rand();
        ret3[1] = 0;
    }
    if(ret3[1]!=100){
        Nokia5110_PrintBMP2(ret3[0], ret3[1] ,retpreto,28,12);
    }


    if(ret4[1]==0){
        ret4[0] = gerar_num_rand();
    }
    if(ret1[1]==36&&ret4[1]==100){
        ret4[0] = gerar_num_rand();
        ret4[1] = 0;
    }
    if(ret4[1]!=100){
        Nokia5110_PrintBMP2(ret4[0], ret4[1] ,retpreto,28,12);
    }


    Nokia5110_PrintBMP2(x_cursor , 46 ,barpreto,28 , 5);

    if(!game_over()){

        Nokia5110_DisplayBuffer();


        ret1[1]++;
        if(ret1[1]==48)
            ret1[1]=0;

        if(ret2[1]!=100)
            ret2[1]++;
        if(ret2[1]==48)
            ret2[1]=0;

        if(ret3[1]!=100)
            ret3[1]++;
        if(ret3[1]==48)
            ret3[1]=0;

        if(ret4[1]!=100)
            ret4[1]++;
        if(ret4[1]==48)
            ret4[1]=0;

        frequencia_timer0 = (SysCtlClockGet() / 1) / (10+(0.1*pontuacao));
        TimerLoadSet(TIMER0_BASE, TIMER_A, frequencia_timer0 -1);

        IntEnable(INT_TIMER0A);
        TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER0_BASE, TIMER_A);

    }else{

        controle_tela++;
        display_game_over();

    }

}

void SW_1_jogo(void){
    if(x_cursor > 0)
        x_cursor -= 28;
}

void SW_2_jogo(void){
    if(x_cursor < 56)
        x_cursor += 28;
}



void display_instrucoes(){
    Nokia5110_Clear();

    Nokia5110_OutString("Utilize os  botoes para se mover e  nao perder  nenhuma nota");
}

void SW_1_instrucoes(){
    controle_tela--;
    display_menu();
}

void SW_2_instrucoes(){
    controle_tela--;
    display_menu();
}



void display_menu(void){
    Nokia5110_Clear();

    Nokia5110_SetCursor(4,0);
    Nokia5110_OutString(tela[controle_tela]);

    Nokia5110_SetCursor(2,2);
    Nokia5110_OutString("JOGAR");

    Nokia5110_SetCursor(2,4);
    Nokia5110_OutString("INSTRUCOES");

}

void SW_1_menu(){

    switch(controle_menu){
    case 0:
        //tela de menu no display
        controle_menu++;
        display_menu();
        Nokia5110_SetCursor(0,2);
        Nokia5110_OutString("--");
        break;
    case 1:
        //JOGAR selecionado
        controle_menu++;
        display_menu();
        Nokia5110_SetCursor(0,4);
        Nokia5110_OutString("--");
        break;
    case 2:
        //INSTRUCOES selecionado
        controle_menu=1;
        display_menu();
        Nokia5110_SetCursor(0,2);
        Nokia5110_OutString("--");
        break;
    default:
        Nokia5110_Clear();
        Nokia5110_OutString("ERROR");
        break;
    }

}

void SW_2_menu(){

    switch(controle_menu){
    case 0:
        //tela de menu no display
        controle_menu++;
        display_menu();
        Nokia5110_SetCursor(0,2);
        Nokia5110_OutString("--");
        break;
    case 1:
        //JOGAR selecionado
        controle_menu=0;
        controle_tela=3;
        pontuacao=0;
        display_jogo();
        break;
    case 2:
        //INSTRUCOES selecionado
        controle_menu=0;
        controle_tela=2;
        display_instrucoes();
        break;
    default:
        Nokia5110_Clear();
        Nokia5110_OutString("ERROR");
        break;
    }

}




void display_tela_inicial(void){
    Nokia5110_Clear();
    Nokia5110_SetCursor(0,2);
    Nokia5110_OutString("PIANO  TILES");

}

void SW_1_tela_inicial(){
    controle_tela++;
    display_menu();
}

void SW_2_tela_inicial(){
    controle_tela++;
    display_menu();
}



//Debouncer do botao 1 (SW_1)
void Timer2IntHandler(void){
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntDisable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    GPIOPinWrite(GPIO_PORTF_BASE, R , 0x00 );

    //Habilitando Interrupcao do botao 1 (SW_1)
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
}

//Debouncer do botao 2 (SW_2)
void Timer1IntHandler(void){
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    GPIOPinWrite(GPIO_PORTF_BASE, B , 0x00);

    //Habilitando Interrupcao do botao 1 (SW_2)
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
}


void IntPortalF (void){

    uint8_t ui8IntSource;
    ui8IntSource = GPIOPinRead(GPIO_PORTF_BASE, SW_1|R|B|G|SW_2 );

    //caso botao 1 (SW_1) seja precionado
    if ((ui8IntSource & 0x10) ==  0x00)
    {
        //limpando e desabilitando a interrupcao do botao 1 (SW_1)
        GPIOIntClear(GPIO_PORTF_BASE, SW_1);

        GPIOPinWrite(GPIO_PORTF_BASE, R , 0xFF );

        //aplicando debouncer
        uint32_t frequencia_timer2;
        frequencia_timer2 = (SysCtlClockGet() / 1) / 5;
        TimerConfigure(TIMER2_BASE, TIMER_CFG_A_ONE_SHOT);
        TimerLoadSet(TIMER2_BASE, TIMER_A, frequencia_timer2 -1);
        IntEnable(INT_TIMER2A);
        TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER2_BASE, TIMER_A);


        //definindo acoes para o botao em cada tela
        switch(controle_tela){
        case 0:
            SW_1_tela_inicial();
            break;
        case 1:
            SW_1_menu();
            break;
        case 2:
            SW_1_instrucoes();
            break;
        case 3:
            SW_1_jogo();
            break;
        case 4:
            SW_1_game_over();
            break;
        default:
            Nokia5110_Clear();
            Nokia5110_OutString("ERROR");
            break;
        }

        GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_4 );

    }

    //caso botao 2 (SW_2) seja precionado
    if ((ui8IntSource & 0x01) ==  0x00)
    {
        //limpando e desabilitando a interrupcao do botao 2 (SW_2)
        GPIOIntClear(GPIO_PORTF_BASE, SW_2);

        GPIOPinWrite(GPIO_PORTF_BASE, B , 0xFF );

        //aplicando debouncer
        uint32_t frequencia_timer1;
        frequencia_timer1 = (SysCtlClockGet() / 1) / 3;
        TimerConfigure(TIMER1_BASE, TIMER_CFG_A_ONE_SHOT);
        TimerLoadSet(TIMER1_BASE, TIMER_A, frequencia_timer1-1);
        IntEnable(INT_TIMER1A);
        TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER1_BASE, TIMER_A);


        //definindo acoes para o botao em cada tela
        switch(controle_tela){
        case 0:
            SW_2_tela_inicial();
            break;
        case 1:
            SW_2_menu();
            break;
        case 2:
            SW_2_instrucoes();
            break;
        case 3:
            SW_2_jogo();
            break;
        case 4:
            SW_2_game_over();
            break;
        default:
            Nokia5110_Clear();
            Nokia5110_OutString("ERROR");
            break;
        }

        GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 );

    }

}



main(void){
    Nokia5110_Init();

    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, R|G|B );

    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    GPIODirModeSet(GPIO_PORTF_BASE, SW_1|SW_2 , GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTF_BASE, SW_1|SW_2 , GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_INT_PIN_0|GPIO_INT_PIN_4 ,GPIO_FALLING_EDGE);
    IntEnable(INT_GPIOF);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0|GPIO_INT_PIN_4);
    IntMasterEnable();

    frequencia_timer0 = (SysCtlClockGet() / 1) / 10;

    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_ONE_SHOT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, frequencia_timer0 -1);

    srand(time(NULL));

    display_tela_inicial();

    while (1)
    {


    }

}
