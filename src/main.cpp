//=====================================
#include "main.h"

//==================================
const uint8_t _CRC[11]  __attribute__((at(0x801C000))) = 
{
    0x55, 0x55, 0x55, 0x55,     // длина ПО
    0x55, 0x55,                 // CRC ПО
    VER_PRODUCT_TYPE,VER_VOLTAGE,VER_PRODUCT_SUBTYPE,VER_ASSEMBLAGE_NUMBER, // версия ПО
    0x00                        // резерв
};
//=====================================
void InitWDT(void);

volatile uint32_t loopsPerSecond = 0; // число циклов главного цикла за последнюю секунду (со сном - число пробуждений)

// Сон до ближайшего прерывания (SysTick раз в 1 мс, CAN, DMA АЦП, таймеры) снижает потребление.
// Для отключения (например, отладка или замер загрузки процессора) закомментируйте строку ниже.
#define MAIN_LOOP_SLEEP

//====================================

int main(void)
{
    __disable_irq();
    core.initialize();   

    config.initialize();
    config.ReadAllFromFlash();
    can.initialize();
    message.initialize();
    Load.initialize();
    adc.initialize();
    __enable_irq();    
    
    InitWDT();
#ifdef MAIN_LOOP_SLEEP
    dbg_low_power_enable(DBG_LOW_POWER_SLEEP); // не терять связь с отладчиком во сне
#endif
    
    uint32_t loopCount = 0;
    uint32_t loopTick = core.getSysTick();
    
    while (1) {   

        adc.handler();
        can.handler();
        message.handler();        
        fwdgt_counter_reload();

        loopCount++;
        if ((core.getSysTick() - loopTick) >= 1000) {
            loopTick += 1000;
            loopsPerSecond = loopCount;
            loopCount = 0;
        }
#ifdef MAIN_LOOP_SLEEP
        __WFI(); // ждём прерывание
#endif
    }
}
//===================================================

void InitWDT(void)
{
    /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
    
    /* wait till IRC40K is ready */
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K)){
    }

    /* confiure FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625 KHz */
    fwdgt_config(0x0FFF,FWDGT_PSC_DIV64);
    
    dbg_periph_enable(DBG_FWDGT_HOLD); // не срабатывает при отладке
    /* After 6.5 seconds to generate a reset */
    fwdgt_enable();
}
//---------------------------------------------------------------


