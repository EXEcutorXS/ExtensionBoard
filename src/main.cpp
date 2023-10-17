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
    
    while (1) {   

        adc.handler();
        can.handler();
        message.handler();        
        Load.handler();  
        //Control_Service(); 
        fwdgt_counter_reload();

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


