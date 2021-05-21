#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"

#define R GPIO_PIN_1
#define B GPIO_PIN_2
#define G GPIO_PIN_3
#define SW_1 GPIO_PIN_4
#define SW_2 GPIO_PIN_0

uint32_t contador = 0;
uint32_t tempo_triac = 0;
uint32_t periodo_pulso_rede;
uint32_t periodo_pulso_triac;
uint32_t delay;

//Timer do pulso rede
void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //acionando o led azul para testar a sincronia dos pulsos
    GPIOPinWrite(GPIO_PORTF_BASE, B , 0xFF );

    //habilitando um timer para apagar o led
    IntEnable(INT_TIMER3A);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER3_BASE, TIMER_A);

    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
}

//Timer do pulso triac
void Timer1IntHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //acionando o led vermelho para testar a sincronia dos pulsos
    GPIOPinWrite(GPIO_PORTF_BASE, R , 0xFF );

    //habilitando um timer para apagar o led
    IntEnable(INT_TIMER4A);
    TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER4_BASE, TIMER_A);

    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
}

//Timer que gera o delay no pulso triac
void Timer2IntHandler(void)
{
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    //habilitando novamente o pulso triac (ja com delay)
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);

    TimerIntDisable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer3IntHandler(void)
{
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);

    //apagando o led azul
    GPIOPinWrite(GPIO_PORTF_BASE, B , 0x00 );

    TimerIntDisable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer4IntHandler(void)
{
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);

    //apagando o led vermelho
    GPIOPinWrite(GPIO_PORTF_BASE, R , 0x00 );

    TimerIntDisable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
}

void IntPortalF (void){

    uint8_t ui8IntSource;
    ui8IntSource = GPIOPinRead(GPIO_PORTF_BASE, SW_1|R|B|G|SW_2 );

    //caso botao 2 (SW_2) seja precionado
    if ((ui8IntSource & 0x01) ==  0x00)
    {

        //inicializando o contador que determinara o atraso dos pulsos triac em relacao aos pulsos rede
        contador = 0;

        //definindo a frequencia que ambos os timers terao
        periodo_pulso_rede = (SysCtlClockGet() / 1) / 1 /*120*/ ;          //*****************

        //os pulsos rede (120Hz) sao iniciados
        TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
        TimerLoadSet(TIMER0_BASE, TIMER_A, periodo_pulso_rede -1);
        IntEnable(INT_TIMER0A);
        TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER0_BASE, TIMER_A);

        //os pulsos TRIAC tambem sao iniciados, tambem em 120Hz
        periodo_pulso_triac = periodo_pulso_rede;
        TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
        TimerLoadSet(TIMER1_BASE, TIMER_A, periodo_pulso_triac -1);
        IntEnable(INT_TIMER1A);
        TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER1_BASE, TIMER_A);

        //setando timers que apagam os leds (apos 100 us)
        uint32_t lightsup;
        lightsup = (SysCtlClockGet() / 1) / 100 /*10000*/ ;                //****************
        TimerConfigure(TIMER3_BASE, TIMER_CFG_A_ONE_SHOT);
        TimerLoadSet(TIMER3_BASE, TIMER_A, lightsup -1);
        TimerConfigure(TIMER4_BASE, TIMER_CFG_A_ONE_SHOT);
        TimerLoadSet(TIMER4_BASE, TIMER_A, lightsup -1);


        GPIOIntClear(GPIO_PORTF_BASE, SW_2);
        //desabilitando a interrupcao do botao 2 (SW_2)
        GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 );

    }

    //caso botao 1 (SW_1) seja precionado
    if ((ui8IntSource & 0x10) ==  0x00)
    {

        //conferindo o incremento que deve ser aplicado no tempo_triac
        contador++;
        if(contador==11)
        {
            contador=0;
        }

        //se for necessario gerar delay
        if(contador != 0 )
        {
            //desabilitando o pulso triac
            TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
            TimerDisable(TIMER1_BASE, TIMER_A);

            //gerando o delay
            delay = periodo_pulso_rede * 0.1;            //delay em funcao do periodo de 1 Hz
            TimerConfigure(TIMER2_BASE, TIMER_CFG_A_ONE_SHOT);
            TimerLoadSet(TIMER2_BASE, TIMER_A, delay -1);
            IntEnable(INT_TIMER2A);
            TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
            TimerEnable(TIMER2_BASE, TIMER_A);
        }

        //calculando o tempo decorrido entre o Pulso Rede e o Pulso TRIAC
        tempo_triac = ((periodo_pulso_rede * 0.1)*contador);


        GPIOIntClear(GPIO_PORTF_BASE, SW_1);
        //desabilitando a interrupcao do botao 1 (SW_1)
        GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_4 );

    }

}

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);

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


    while(1)
        {

        }

}
