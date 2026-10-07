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
#include "config.h"
#include "flash_tools.h"
#include "paramsname.h"
#include "can.h"
//--------------------------------------------------

//-----------------------------------------------------

Config config; 
//----------------------------------------------------
void Config::initialize(void)
{
    flash.initialize();
}
//----------------------------------
void Config::Erase(void)
{
    for (uint8_t i=0; i<(uint8_t)((CONFIG_END_ADDR-CONFIG_START_ADDR)/CONFIG_PAGE_SIZE);i++) {
        flash.Erase(CONFIG_START_ADDR+i*CONFIG_PAGE_SIZE);
    }
    AddressWrite=CONFIG_START_ADDR;
}

//-----------------------------------------------------
void Config::Unlock(void)
{
    flash.Unlock();         
}
//-----------------------------------------------------
void Config::Lock(void)
{
    flash.Lock();          
}  
//-----------------------------------------------------
//чтение одного параметра из FLASH
uint32_t Config::ReadParameterFlash(uint32_t AName)
{
    for (uint32_t Address=CONFIG_START_ADDR; Address<CONFIG_END_ADDR; Address=Address+8) {
        uint32_t V=flash.ReadValue(Address);
        if (V==0xFFFFFFFF ) return 0xFFFFFFFF;                              //массив закончился
        if (V==AName) return flash.ReadValue(Address+4);
    }
    return 0xFFFFFFFF;
}
//-----------------------------------------------------
//запись одного параметра во flash
void Config::WriteParameterFlash(uint32_t AName, uint32_t AValue)
{
    if (AddressWrite>=CONFIG_START_ADDR && AddressWrite<=CONFIG_END_ADDR-8) {
        flash.WriteValue(AddressWrite,AName);  AddressWrite = AddressWrite+4;
        flash.WriteValue(AddressWrite,AValue); AddressWrite = AddressWrite+4;
    }
} 
//-----------------------------------------------------
//чтение всех параметров из flash в ОЗУ
void Config::ReadAllFromFlash(void)
{
    uint32_t V;

    V=ReadParameterFlash(PAR_RVC_INSTANCE);
    if (V < 127) Instance=(uint8_t)V; else Instance=0;
    
    V=ReadParameterFlash(PAR_CAN_ADDRESS);
    if (V < 7) can.idAddress = (uint8_t)V; else can.idAddress = 0;

    V=ReadParameterFlash(PAR_ID1);
    if (V==0xFFFFFFFF) { ID[0]=0;        ID[1]=0;            ID[2]=0;            ID[3]=0;      }
    else               { ID[0]=V&0xFF;   ID[1]=(V>>8)&0xFF;  ID[2]=(V>>16)&0xFF; ID[3]=(V>>24)&0xFF; }
    V=ReadParameterFlash(PAR_ID2);
    if (V==0xFFFFFFFF) { ID[4]=0;        ID[5]=0;            ID[6]=0;            ID[7]=0;      }
    else               { ID[4]=V&0xFF;   ID[5]=(V>>8)&0xFF;  ID[6]=(V>>16)&0xFF; ID[7]=(V>>24)&0xFF; }
    V=ReadParameterFlash(PAR_ID3);
    if (V==0xFFFFFFFF) { ID[8]=0;        ID[9]=0;            ID[10]=0;            ID[11]=0;      }
    else               { ID[8]=V&0xFF;   ID[9]=(V>>8)&0xFF;  ID[10]=(V>>16)&0xFF; ID[11]=(V>>24)&0xFF; }
}
//-----------------------------------------------------
//чтение одного параметра из ОЗУ
uint32_t Config::ReadFromRAM(uint32_t AName)
{
    if (AName==PAR_RVC_INSTANCE) return Instance;
    if (AName==PAR_CAN_ADDRESS) return can.idAddress;   
    if (AName==PAR_ID1         ) return (uint32_t)((ID[3]<<24)+(ID[2]<<16)+(ID[1]<<8)+ID[0]);
    if (AName==PAR_ID2         ) return (uint32_t)((ID[7]<<24)+(ID[6]<<16)+(ID[5]<<8)+ID[4]);
    if (AName==PAR_ID3         ) return (uint32_t)((ID[11]<<24)+(ID[10]<<16)+(ID[9]<<8)+ID[8]);
    
    return 0xFFFFFFFF;
}
//-----------------------------------------------------
//запись одного параметра в ОЗУ
void Config::WriteToRAM(uint32_t AName, uint32_t AValue)
{
    if (AName==PAR_RVC_INSTANCE) {
        if (AValue <= 255) Instance=AValue;
        else Instance=0;

    }
    if (AName==PAR_CAN_ADDRESS) {
        if (AValue < 7) can.idAddress=AValue;
        else can.idAddress=0;
    }
    if (AName==PAR_ID1                     ) { ID[3]=(AValue>>24)&0xFF;ID[2]=(AValue>>16)&0xFF;ID[1]=(AValue>>8)&0xFF;ID[0]=AValue&0xFF; }
    if (AName==PAR_ID2                     ) { ID[7]=(AValue>>24)&0xFF;ID[6]=(AValue>>16)&0xFF;ID[5]=(AValue>>8)&0xFF;ID[4]=AValue&0xFF; }
    if (AName==PAR_ID3                     ) { ID[11]=(AValue>>24)&0xFF;ID[10]=(AValue>>16)&0xFF;ID[9]=(AValue>>8)&0xFF;ID[8]=AValue&0xFF; }

}
//-----------------------------------------------------
//запись ВСЕХ параметров во flash
void Config::WriteAllToFlash(void)
{
    Erase();                                                                      //стереть настройки
    Unlock();

    WriteParameterFlash(PAR_RVC_INSTANCE, Instance);    
    WriteParameterFlash(PAR_CAN_ADDRESS, can.idAddress);     
    WriteParameterFlash(PAR_ID1, (uint32_t)((ID[3]<<24)+(ID[2]<<16)+(ID[1]<<8)+ID[0]));
    WriteParameterFlash(PAR_ID2, (uint32_t)((ID[7]<<24)+(ID[6]<<16)+(ID[5]<<8)+ID[4]));
    WriteParameterFlash(PAR_ID3, (uint32_t)((ID[11]<<24)+(ID[10]<<16)+(ID[9]<<8)+ID[8]));

    Lock();
}
//============================================================================================
