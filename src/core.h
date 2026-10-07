/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/

#ifndef __CORE_H
#define __CORE_H

#include "gd32f30x.h"

//----------------------------------------------

class Core
{
    public:
        Core(void);
        void initialize(void);
        void SystemCoreClockInit(void);
        void systick_config(void);
        uint32_t getSysTick(void);
        volatile uint32_t msTick;
    
    private:
        
        
};
extern Core core;

#endif /* __CORE_H */

