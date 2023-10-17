/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/

#include "core.h"
#include "version.h"

//-------------------------
Core core;
//--------------
Core::Core(void)
{

}
//---------------------------------
void Core::initialize(void)
{
    delay = 0;
    msTick = 0;
    SystemCoreClockInit();
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, MAIN_PROGRAM_START_ADDRESS - FLASH_BASE);
    pmu_lvd_select(PMU_LVDT_4); // PMU_LVDT_4: voltage threshold is 2.7V
    pmu_ldo_output_select(PMU_LDOVS_LOW);
}
//---------------------------------------
void Core::SystemCoreClockInit(void) // на 16 Мгц
{  
    rcu_osci_on(RCU_HXTAL);
    //rcu_osci_bypass_mode_enable(RCU_HXTAL);
    rcu_lxtal_drive_capability_config(RCU_LXTAL_HIGHDRI); // RCU_LXTAL_LOWDRI // RCU_LXTAL_HIGHDRI

    rcu_periph_clock_enable(RCU_PMU);
    pmu_ldo_output_select(PMU_LDOVS_HIGH);
    
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV1);
      
    if (rcu_osci_stab_wait(RCU_HXTAL) == SUCCESS) { // если внешний кристал
        rcu_pllpresel_config(RCU_PLLPRESRC_HXTAL); // RCU_PLLPRESRC_HXTAL / RCU_PLLPRESRC_IRC48M 
        rcu_predv0_config(RCU_PREDV0_DIV2); // HXTAL_VALUE/2 
        rcu_system_clock_source_config(RCU_CKSYSSRC_HXTAL);// RCU_CKSYSSRC_IRC8M / RCU_CKSYSSRC_HXTAL / RCU_CKSYSSRC_PLL // работаем на частоте кварца
        //rcu_pll_config(RCU_PLLSRC_HXTAL_IRC48M, RCU_PLL_MUL6); // RCU_PLLSRC_IRC8M_DIV2 / RCU_PLLSRC_HXTAL_IRC48M            
        //rcu_hxtal_clock_monitor_enable();   
    } else {
        rcu_osci_off(RCU_HXTAL);
        rcu_osci_bypass_mode_disable(RCU_HXTAL);
        rcu_osci_on(RCU_IRC8M);
        if (rcu_osci_stab_wait(RCU_IRC8M) == SUCCESS) { // IRC8M_VALUE/2 * 4 = 16 000 000
            rcu_pll_config(RCU_PLLSRC_IRC8M_DIV2, RCU_PLL_MUL4); // RCU_PLLSRC_IRC8M_DIV2 / RCU_PLLSRC_HXTAL_IRC48M
            rcu_osci_on(RCU_PLL_CK); 
            if (rcu_osci_stab_wait(RCU_PLL_CK) == SUCCESS) {
                rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);// RCU_CKSYSSRC_IRC8M / RCU_CKSYSSRC_HXTAL / RCU_CKSYSSRC_PLL
            } else {
                rcu_osci_off(RCU_PLL_CK); 
                rcu_system_clock_source_config(RCU_CKSYSSRC_IRC8M);// RCU_CKSYSSRC_IRC8M / RCU_CKSYSSRC_HXTAL / RCU_CKSYSSRC_PLL
                // не получилось на 16 - работаем на 8
            }
        }
        rcu_periph_clock_enable(RCU_GPIOD); 
        rcu_periph_clock_enable(RCU_AF);
        rcu_periph_clock_enable(RCU_PMU);
        gpio_pin_remap_config(GPIO_PD01_REMAP,ENABLE);
        gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0); // инициализация неиспользуемых выводов 
        gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_1); // инициализация неиспользуемых выводов 
        gpio_bit_reset(GPIOD, GPIO_PIN_0);
        gpio_bit_reset(GPIOD, GPIO_PIN_1);
    }
    
    pmu_highdriver_mode_enable();
    pmu_highdriver_switch_select(PMU_HIGHDR_SWITCH_EN);

     
    SystemCoreClockUpdate();
    
    systick_config();
    
}
//===============================================
/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Core::systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 1U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void Core::delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Core::delay_decrement(void)
{
    if (0U != delay){
        delay--;
    }
}
//=================================

uint32_t Core::getSysTick(void)
{
    return msTick;
}
//=====================================

extern "C" void SysTick_Handler(void)
{
    core.msTick++;
}
//=====================================
extern "C" void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    NVIC_SystemReset();
    while (1);
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern "C" void PendSV_Handler(void)
{
}
//=========================================================
