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
#define PWM_FREQ     25000

#define LOAD1        1 
#define LOAD2        2
#define LOAD3        3

#define LOAD1_PORT        GPIOA 
#define LOAD1_PIN         GPIO_PIN_7
#define LOAD2_PORT        GPIOB
#define LOAD2_PIN         GPIO_PIN_0
#define LOAD3_PORT        GPIOB
#define LOAD3_PIN         GPIO_PIN_1

#define PWM_TIMER         TIMER2
#define RCU_PWM_TIMER     RCU_TIMER2

//-------------------------------------------------------------------
class LoadControl
{
    public:
        void initialize(void);
        void handler(void);
        void setPower(uint8_t loadNumb, uint16_t promile);
        uint16_t getPower(uint8_t loadNumb);
        void Deinit(void);
    private:
        uint32_t MaxPwmPulse;
        uint32_t Promile[3];

};
extern LoadControl Load;           
//-----------------------------------------------------------------
#endif /* __LOAD_H */

