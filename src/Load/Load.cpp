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
    rcu_periph_clock_enable(RCU_SOFT_PWM_TIMER);
    
    // Частичный ремап TIMER2: CH0 -> PB4, CH1 -> PB5, CH2 -> PB0, CH3 -> PB1 (CH0 не используется)
    gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);
    // LOAD1 на старой плате (PA7): программный ШИМ, управляется из TIMER3_IRQHandler
    gpio_init(LOAD1_SOFT_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LOAD1_SOFT_PIN);
    gpio_bit_reset(LOAD1_SOFT_PORT, LOAD1_SOFT_PIN);
    // LOAD1 на новой плате (PB5): аппаратный ШИМ
    gpio_init(LOAD1_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LOAD1_PIN); 
    gpio_bit_reset(LOAD1_PORT, LOAD1_PIN);
    gpio_init(LOAD2_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LOAD2_PIN); 
    gpio_bit_reset(LOAD2_PORT, LOAD2_PIN);
    gpio_init(LOAD3_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LOAD3_PIN); 
    gpio_bit_reset(LOAD3_PORT, LOAD3_PIN);
    
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
    
           
    timer_auto_reload_shadow_enable(PWM_TIMER);
    timer_enable(PWM_TIMER);
    timer_channel_output_state_config(PWM_TIMER, TIMER_CH_1, TIMER_CCX_ENABLE);
    timer_channel_output_state_config(PWM_TIMER, TIMER_CH_2, TIMER_CCX_ENABLE); 
    timer_channel_output_state_config(PWM_TIMER, TIMER_CH_3, TIMER_CCX_ENABLE);

    // Таймер программного ШИМ LOAD1 (PA7): переполнение - включить выход, совпадение с каналом 0 - выключить
    timer_deinit(SOFT_PWM_TIMER);
    MaxSoftPulse = SystemCoreClock/(timer_initparam.prescaler+1)/LOAD1_SOFT_PWM_FREQ;
    timer_initparam.period = (uint16_t)MaxSoftPulse;
    timer_init(SOFT_PWM_TIMER, &timer_initparam);
    timer_channel_output_pulse_value_config(SOFT_PWM_TIMER, TIMER_CH_0, 0);
    nvic_irq_enable(TIMER3_IRQn, 0, 0); // прерывания включаются в setPower
    timer_enable(SOFT_PWM_TIMER);

    // Все выходы в безопасном состоянии (0). Драйверы затворов выключены, включатся первой ненулевой командой
    gpio_init(DRV_EN_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, DRV_EN_PIN);
    gpio_bit_reset(DRV_EN_PORT, DRV_EN_PIN);


}
//-----------------------------------------------------------------
// Программный ШИМ LOAD1 (PA7) с точностью 1%
void LoadControl::setSoftPwm1(uint16_t promile)
{
    uint32_t percent = (promile + 5) / 10;                    // 0..100
    uint16_t pulse = (uint32_t)MaxSoftPulse * percent / 100;
    timer_channel_output_pulse_value_config(SOFT_PWM_TIMER, TIMER_CH_0, pulse);
    if ((pulse < LOAD1_SOFT_PWM_MIN_TICKS) || (pulse > (MaxSoftPulse - LOAD1_SOFT_PWM_MIN_TICKS))) {
        // 0% / 100% (или слишком короткий импульс или пауза) - постоянный уровень, без прерываний
        timer_interrupt_disable(SOFT_PWM_TIMER, TIMER_INT_UP | TIMER_INT_CH0);
        timer_interrupt_flag_clear(SOFT_PWM_TIMER, TIMER_INT_FLAG_UP | TIMER_INT_FLAG_CH0);
        if (pulse >= (MaxSoftPulse/2)) gpio_bit_set(LOAD1_SOFT_PORT, LOAD1_SOFT_PIN);
        else gpio_bit_reset(LOAD1_SOFT_PORT, LOAD1_SOFT_PIN);
    } else {
        timer_interrupt_flag_clear(SOFT_PWM_TIMER, TIMER_INT_FLAG_UP | TIMER_INT_FLAG_CH0); // сбросить устаревшие флаги
        timer_interrupt_enable(SOFT_PWM_TIMER, TIMER_INT_UP | TIMER_INT_CH0);
    }
}
//-----------------------------------------------------------------
void LoadControl::setPower(uint8_t loadNumb, uint16_t promile){   
    if ((loadNumb < 1) || (loadNumb > 3)) return;
    if (promile > 1000) promile = 1000;
    Promile[loadNumb-1] = promile;
    if (promile > 0) gpio_bit_set(DRV_EN_PORT, DRV_EN_PIN); // драйверы включаем до того, как появится выходной сигнал
    uint16_t pulse = ((uint32_t)MaxPwmPulse*promile+500)/1000;  
    if (pulse < 10) pulse = 0;
    if (pulse > MaxPwmPulse) pulse = MaxPwmPulse;
    if (loadNumb == 1) {  
        timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_1, pulse); // аппаратный ШИМ (PB5)
        setSoftPwm1(promile);                                                  // программный ШИМ (PA7)
    }
    if (loadNumb == 2) {  
        timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_2, pulse);
    }
    if (loadNumb == 3) {  
        timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_3, pulse);
    }
    // Выходного сигнала нет ни на одном канале - драйверы отключаем после того, как выходы уже обнулены
    if ((Promile[0] == 0) && (Promile[1] == 0) && (Promile[2] == 0)) gpio_bit_reset(DRV_EN_PORT, DRV_EN_PIN);

}
//-------------------------------------------------------------------------------
uint16_t LoadControl::getPower(uint8_t loadNumb)
{
    uint16_t promile = 0;
    uint32_t pulse;
    if (loadNumb == 1) {
        pulse = timer_channel_capture_value_register_read(PWM_TIMER, TIMER_CH_1);
    } else if (loadNumb == 2) {  
        pulse = timer_channel_capture_value_register_read(PWM_TIMER, TIMER_CH_2); 
    } else if (loadNumb == 3) {  
        pulse = timer_channel_capture_value_register_read(PWM_TIMER, TIMER_CH_3); 
    } else {
        return 0;
    }
    promile = (uint32_t)(((uint32_t)pulse*1000)+((MaxPwmPulse+1)/2))/MaxPwmPulse;
    if (promile > 1000) promile = 1000;
    uint32_t savedPromile = Promile[loadNumb-1];
    if ((promile >= (savedPromile-1))&&(promile <= (savedPromile+1))) promile = savedPromile; // проблема округления
    return promile;
}
//----------------------------------------------
// Программный ШИМ LOAD1 (PA7): начало периода - включить выход, совпадение с каналом 0 - выключить
extern "C" void TIMER3_IRQHandler(void)
{
    uint32_t flags = TIMER_INTF(SOFT_PWM_TIMER);
    if (flags & TIMER_INTF_UPIF) {
        TIMER_INTF(SOFT_PWM_TIMER) = ~(uint32_t)TIMER_INTF_UPIF;
        GPIO_BOP(LOAD1_SOFT_PORT) = LOAD1_SOFT_PIN;
    }
    if (flags & TIMER_INTF_CH0IF) {
        TIMER_INTF(SOFT_PWM_TIMER) = ~(uint32_t)TIMER_INTF_CH0IF;
        GPIO_BC(LOAD1_SOFT_PORT) = LOAD1_SOFT_PIN;
    }
}
//=============================================
