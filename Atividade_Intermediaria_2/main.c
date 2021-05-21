#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
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

double pi = 3.14159265358;

int hora=0, minuto=0, segundo=0;
char horario = 0;
char hhmmss[9] = {0,0,0,0,0,0,0,0,0};
char h[3] = {0,0,0};
char m[3] = {0,0,0};
char s[3] = {0,0,0};

bool digital = true;

uint32_t periodo_timer;
uint32_t periodo_segundo;

int teste = 0;

//funcão para imprimir o horario no display
void Time_display_digital(void){

    Nokia5110_Clear();

    Nokia5110_SetCursor(6,2);
    Nokia5110_OutUDec(segundo);
    Nokia5110_SetCursor(3,2);
    Nokia5110_OutUDec(minuto);
    Nokia5110_SetCursor(0,2);
    Nokia5110_OutUDec(hora);
    Nokia5110_SetCursor(5,2);
    Nokia5110_OutChar(':');
    Nokia5110_SetCursor(8,2);
    Nokia5110_OutChar(':');

}

void Time_display_analogico(){

    Nokia5110_Clear();

    Nokia5110_PrintBMP2(0,0, Circulo ,47,47);
    Nokia5110_PrintBMP2(21,21,Ponto,5,5);
    Nokia5110_DisplayBuffer();

    int hora_analogico;

    if(hora>11){
        hora_analogico = hora-12;
    }else{
        hora_analogico = hora;
    }

    //definindo a que distancia do centro os pontos que representam os horarios serão printados
    int raio_hora = 7;
    int raio_minuto = 11;
    int raio_segundo = 15;

    double angulo_hora = 90 - hora_analogico*30;
    double angulo_minuto = 90 - minuto*6;
    double angulo_segundo = 90 - segundo*6;

    double x_hora = rint(raio_hora * cos((angulo_hora/180)*pi));
    double y_hora = rint(raio_hora * sin((angulo_hora/180)*pi));
    Nokia5110_PrintBMP2(22+x_hora, 22-y_hora ,Hora,3,3);

    double x_minuto = rint(raio_minuto * cos((angulo_minuto/180)*pi));
    double y_minuto = rint(raio_minuto * sin((angulo_minuto/180)*pi));
    Nokia5110_PrintBMP2(22+x_minuto, 22-y_minuto, Hora,3,3);

    double x_segundo = rint(raio_segundo * cos((angulo_segundo/180)*pi));
    double y_segundo = rint(raio_segundo * sin((angulo_segundo/180)*pi));
    Nokia5110_PrintBMP2(22+x_segundo, 22-y_segundo, Hora,3,3);

    Nokia5110_DisplayBuffer();

}

//funcao para ler o gorario passado pela UART
void Time_Reader(void){

    uint16_t controle = 0;

    while(UARTCharsAvail(UART0_BASE)) //enquando houver caracteres
    {
        horario = UARTCharGet(UART0_BASE);
        if(horario!=0x0a){
            //Nokia5110_OutChar(horario);
            hhmmss[controle]=horario;
        }
        controle++;
    }

    //repassando os valores numericos do horario
    h[0]=hhmmss[0]; h[1]=hhmmss[1];
    m[0]=hhmmss[3]; m[1]=hhmmss[4];
    s[0]=hhmmss[6]; s[1]=hhmmss[7];

    //salvando o horario em variáveis numericas
    hora = atoi(h);
    minuto = atoi(m);
    segundo = atoi(s);

    //lidando com o caso de numeros incompatoveis com o proposito do relogio
    if(hora>23)
        hora=0;
    if(minuto>59)
        minuto=0;
    if(segundo>59)
        segundo=0;

}

//Interrupção causada pela UART
void UARTIntHandler(void)
{

    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, ui32Status);

    if(UARTCharsAvail(UART0_BASE)) //se houverem caracteres
        Time_Reader();

    TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerDisable(TIMER1_BASE, TIMER_A);

    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);


    if(digital==true){
        Time_display_digital();
    }else{
        Time_display_analogico();
    }

}

//timer para promover a troca do mostrador digital para o analogico e vice versa
void Timer0IntHandler(void)
{

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //led adicionado para acompanhar periodo do timer0
    uint8_t ui8IntSource;
    ui8IntSource = GPIOPinRead(GPIO_PORTF_BASE,  GPIO_PIN_2);
    if(ui8IntSource==0){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    }else{
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }

    //promovendo a troca do mostrador digital para o analogio e vice versa
    if(digital==true){
        digital=false;
        Time_display_analogico();
    }else{
        digital=true;
        Time_display_digital();
    }

}

//timer para promover a atualizacao do horario de 1 em 1 segundo
void Timer1IntHandler(void){

    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //led adicionado para acompanhar periodo do timer1
    uint8_t ui8IntSource;
    ui8IntSource = GPIOPinRead(GPIO_PORTF_BASE,  GPIO_PIN_3);
    if(ui8IntSource==0){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    }else{
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    }

    segundo++;

    if(segundo>=60){
        segundo = 0;
        minuto++;
    }if(minuto>=60){
        segundo = 0;
        minuto = 0;
        hora++;
    }if(hora>=24){
        segundo = 0;
        minuto = 0;
        hora = 0;
    }

    if(digital==true){
        Time_display_digital();
    }else{
        Time_display_analogico();
    }

}



int main(void){

    Nokia5110_Init();
    Nokia5110_Clear();

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    //definindo o tempo de permanencia em cada tela (10s)
    periodo_timer = (SysCtlClockGet() / 1) / 0.1;
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, periodo_timer);


    //Timer para atualizar o horario
    periodo_segundo = (SysCtlClockGet() / 1) / 1;
    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, periodo_segundo);


    //habilitando UARt e Timer0
    IntMasterEnable();
    IntEnable(INT_UART0);
    IntEnable(INT_TIMER0A);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    //habilitando Timer1
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);


    //Imprimindo um primeiro horário
    hora=22;
    minuto=41;
    segundo=23;

    Time_display_digital();

    while (1)
    {

    }

}
