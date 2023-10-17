/********************************************************************
* ООО Адверс
* Самара
* 
* Программисты: 
* 
* 01.04.2022
* Описание:
********************************************************************/
/* Includes -------------------------------------------------------*/
#include "can.h"
#include "core.h"
#include "Load.h"

//==================================================================
//--------------------------

LoadControl Load;  
//-------------------------------------------------------------------

void LoadControl::initialize(void)
{                                         
    timer_parameter_struct timer_initparam = {0};
    timer_oc_parameter_struct timer_ocinitparam = {0};
    Promile[0] = 0;
    Promile[1] = 0;
    Promile[2] = 0;
    // Enable the Clock
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_PMU);
    rcu_periph_clock_enable(RCU_PWM_TIMER);
    
//    gpio_init(LOAD1_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LOAD1_PIN); 
    gpio_init(LOAD1_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LOAD1_PIN); 
    gpio_bit_reset(LOAD1_PORT, LOAD1_PIN);
    gpio_init(LOAD2_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LOAD2_PIN); 
    gpio_bit_reset(LOAD2_PORT, LOAD2_PIN);
    gpio_init(LOAD3_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LOAD3_PIN); 
    gpio_bit_reset(LOAD3_PORT, LOAD3_PIN);
    //gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);
    
    timer_deinit(PWM_TIMER);
    timer_initparam.prescaler         = 0; // 0 for 16Mhz  
    timer_initparam.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initparam.counterdirection  = TIMER_COUNTER_UP;
    MaxPwmPulse = SystemCoreClock/(timer_initparam.prescaler+1)/PWM_FREQ;
    timer_initparam.period            = (uint16_t)MaxPwmPulse;
    timer_initparam.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initparam.repetitioncounter = 0;
    timer_init(PWM_TIMER, &timer_initparam);

    timer_auto_reload_shadow_disable(PWM_TIMER);
    timer_master_slave_mode_config(PWM_TIMER,TIMER_MASTER_SLAVE_MODE_DISABLE);
    // channel 1
    timer_ocinitparam.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitparam.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocinitparam.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitparam.ocnpolarity  = TIMER_OCN_POLARITY_LOW;
    timer_ocinitparam.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocinitparam.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;
    // channel 1
    timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);
    timer_channel_output_config(PWM_TIMER, TIMER_CH_1, &timer_ocinitparam);
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_1, 0);
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_fast_config(PWM_TIMER, TIMER_CH_1, TIMER_OC_FAST_DISABLE);
    
    // channel 2
    timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
    timer_channel_output_config(PWM_TIMER, TIMER_CH_2, &timer_ocinitparam);
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_2, 0);
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_fast_config(PWM_TIMER, TIMER_CH_2, TIMER_OC_FAST_DISABLE);
       
    // channel 3
    timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);
    timer_channel_output_config(PWM_TIMER, TIMER_CH_3, &timer_ocinitparam);
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_3, 0);
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_3, TIMER_OC_MODE_PWM0);
    timer_channel_output_fast_config(PWM_TIMER, TIMER_CH_3, TIMER_OC_FAST_DISABLE);
    
           
    nvic_irq_enable(TIMER2_IRQn, 0, 1); 
    timer_auto_reload_shadow_enable(PWM_TIMER);
    timer_enable(PWM_TIMER);
    timer_channel_output_state_config(PWM_TIMER, TIMER_CH_1, TIMER_CCX_ENABLE);
    timer_channel_output_state_config(PWM_TIMER, TIMER_CH_2, TIMER_CCX_ENABLE); 
    timer_channel_output_state_config(PWM_TIMER, TIMER_CH_3, TIMER_CCX_ENABLE);


}
//----------------------------------------------------------------
void LoadControl::Deinit(void)
{
    gpio_init(LOAD1_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, LOAD1_PIN); 
    gpio_init(LOAD2_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, LOAD2_PIN); 
    gpio_init(LOAD3_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, LOAD3_PIN); 
}
//-------------------------------------------------------------------
void LoadControl::handler(void)
{
}    
             
//-----------------------------------------------------------------
void LoadControl::setPower(uint8_t loadNumb, uint16_t promile)
{   
    if (promile > 1000) promile = 0;
    if ((loadNumb > 0)&&(loadNumb <= 3)) Promile[loadNumb-1] = promile;
    uint16_t pulse = ((uint32_t)MaxPwmPulse*promile+500)/1000;  
    if (pulse < 10) pulse = 0;
    if (pulse > MaxPwmPulse) pulse = MaxPwmPulse;
    if (loadNumb == 1) {  
        timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_1, pulse);
        if (pulse >= (MaxPwmPulse/2)) gpio_bit_set(LOAD1_PORT, LOAD1_PIN);
        else gpio_bit_reset(LOAD1_PORT, LOAD1_PIN);
    }
    if (loadNumb == 2) {  
        timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_2, pulse);
    }
    if (loadNumb == 3) {  
        timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_3, pulse);
    }

}
//-------------------------------------------------------------------------------
uint16_t LoadControl::getPower(uint8_t loadNumb)
{
    uint16_t promile = 0;
    uint32_t pulse;
    if (loadNumb == 1) {
        pulse = timer_channel_capture_value_register_read(PWM_TIMER, TIMER_CH_1);
        if (pulse >= (MaxPwmPulse/2)) pulse = MaxPwmPulse;
        else pulse = 0;
    }
    if (loadNumb == 2) {  
        pulse = timer_channel_capture_value_register_read(PWM_TIMER, TIMER_CH_2); 
    }
    if (loadNumb == 3) {  
        pulse = timer_channel_capture_value_register_read(PWM_TIMER, TIMER_CH_3); 
    }
    promile = (uint32_t)(((uint32_t)pulse*1000)+((MaxPwmPulse+1)/2))/MaxPwmPulse;
    if (promile > 1000) promile = 1000;
    uint32_t savedPromile = Promile[loadNumb-1];
    if ((promile >= (savedPromile-1))&&(promile <= (savedPromile+1))) promile = savedPromile; // проблема округления
    return promile;
}
//----------------------------------------------

extern "C" void TIMER2_IRQHandler(void)
{
}
//=============================================

