/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H
/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

#define CONFIG_PAGE_SIZE  0x800
#define CONFIG_START_ADDR 0x801E800                                //с какого адреса записываются настройки
#define CONFIG_END_ADDR   CONFIG_START_ADDR + CONFIG_PAGE_SIZE     //до какого адреса записываются настройки
//===========================================================================================================
class Config
{
    public:
        void initialize(void);
        void Erase(void);                                                  //стереть страницу параметров работы
        void Unlock(void);                                                 
        void Lock(void);
        uint32_t ReadParameterFlash(uint32_t AName);                       //поиск и считывание определенного значения параметра работы
        void WriteParameterFlash(uint32_t AName, uint32_t AValue);         //запись имени и значения параметра
        void ReadAllFromFlash(void);                                       //чтение всех настроек из flash
        uint32_t ReadFromRAM(uint32_t AName);
        void WriteToRAM(uint32_t AName, uint32_t AValue);                  //запись одного параметра в ОЗУ
        void WriteAllToFlash(void);                                        //запись ВСЕХ параметров работы
    private:

        uint8_t Instance;
        uint32_t AddressWrite;
        uint8_t ID[12];   //серийный номер 
};

extern Config config;
//===================================================================

#endif /* __CONFIG_H */

