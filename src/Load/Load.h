/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/
#ifndef __LOAD_H
#define __LOAD_H
/* Includes -------------------------------------------------------*/
#include "gd32f30x.h"
//==================================================================
#define PWM_FREQ     25000          // частота аппаратного ШИМ (TIMER2)

// LOAD1 выведен на две ноги одновременно (одна прошивка для старых и новых плат):
//  - PB5 (TIMER2_CH1 с частичным ремапом) - аппаратный ШИМ, новая плата;
//  - PA7 - программный ШИМ по прерываниям TIMER3, старая плата (на этой ноге аппаратный ШИМ не работает).
// Точность программного ШИМ 1%.
#define LOAD1_SOFT_PWM_FREQ       2000
// Импульсы и паузы короче этого значения (в тактах таймера) прерывания отработать не успевают -
// в таком случае выход остаётся постоянно выключенным или включенным (при 2 кГц это меньше 1%).
#define LOAD1_SOFT_PWM_MIN_TICKS  64

#define LOAD1        1 
#define LOAD2        2
#define LOAD3        3

#define LOAD1_PORT        GPIOB          // LOAD1, аппаратный ШИМ: TIMER2_CH1 с частичным ремапом (PB5)
#define LOAD1_PIN         GPIO_PIN_5
#define LOAD1_SOFT_PORT   GPIOA          // LOAD1, программный ШИМ (PA7)
#define LOAD1_SOFT_PIN    GPIO_PIN_7
#define LOAD2_PORT        GPIOB
#define LOAD2_PIN         GPIO_PIN_0
#define LOAD3_PORT        GPIOB
#define LOAD3_PIN         GPIO_PIN_1

#define DRV_EN_PORT       GPIOB          // включение драйверов затворов
#define DRV_EN_PIN        GPIO_PIN_4

#define PWM_TIMER         TIMER2
#define RCU_PWM_TIMER     RCU_TIMER2

#define SOFT_PWM_TIMER      TIMER3
#define RCU_SOFT_PWM_TIMER  RCU_TIMER3

//-------------------------------------------------------------------
class LoadControl
{
    public:
        void initialize(void);
        void setPower(uint8_t loadNumb, uint16_t promile);
        uint16_t getPower(uint8_t loadNumb);
    private:
        void setSoftPwm1(uint16_t promile);
        uint32_t MaxPwmPulse;      // период аппаратного ШИМ, такты
        uint32_t MaxSoftPulse;     // период программного ШИМ LOAD1 (PA7), такты
        uint32_t Promile[3];

};
extern LoadControl Load;           
//-----------------------------------------------------------------
#endif /* __LOAD_H */

