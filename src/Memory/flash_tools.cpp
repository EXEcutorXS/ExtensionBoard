/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "flash_tools.h"
//============================================================================
Flash flash;
//-----------------------------------------------------
void Flash::initialize(void)
{
    // Next commands may be used in SysClock initialization function. In this case using of FLASH_Init is not obligatorily 
    ///FLASH_PrefetchBufferCmd(ENABLE);                                 // Enable Prefetch Buffer
    ///FLASH_SetLatency( FLASH_Latency_1);                              // Flash 2 wait state
}

//-----------------------------------------------------
void Flash::Erase(uint32_t APageAddress)
{
    fmc_unlock();                      
    fmc_page_erase(APageAddress);       
    fmc_lock();                       
    fmc_bank0_ready_wait(FMC_TIMEOUT_COUNT);
}
//-----------------------------------------------------
uint32_t Flash::ReadValue(uint32_t AAddress)
{       
    uint32_t V;
    V= *(__IO uint32_t*)AAddress;
    return V;
}
//-----------------------------------------------------
void Flash::WriteValue(uint32_t AAddress, uint32_t AValue)
{    
    fmc_word_program(AAddress, AValue);      
}
//============================================================
void Flash::Unlock(void)
{
    fmc_unlock(); 
}
//------------------
void Flash::Lock(void)
{
    fmc_lock(); 
}
//==============================

