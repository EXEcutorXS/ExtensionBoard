/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/
/* Includes -------------------------------------------------------*/
#include "can.h"
#include "core.h"
#include "Can_Messages.h"
#include "Load.h"
#include "version.h"
#include "config.h"
#include "ADCmodule.h"
//-----------------------------------------------------
CanMessage message;
//=======================================================
void CanMessage::initialize(void)
{
    TimeTransPgn15 = 1000;
    TimeTransPgn44 = 2000;
    TimeTransPgn45 = 3000;
}
//------------------------------------------

void CanMessage::handler(void)
{
    
    for(uint8_t i=0; i<MSG_BUF_SIZE; i++) {
        if (can.MsgBuf[i].Received_f) {  
            // обработка принятых сообщений
            uint8_t AD[8];
            for(uint8_t k=0; k<8; k++) {
                AD[k] = can.MsgBuf[i].Msg.rx_data[k];
            }
            Process(can.MsgBuf[i].Msg.rx_efid, AD[0], AD[1], AD[2], AD[3], AD[4], AD[5], AD[6], AD[7]);
            can.MsgBuf[i].Received_f = 0;
            //return;
        }
    }
    if (adc.ready_f) // не работаем как кнопка, пока переходные процессы не пройдут
    {
        for(uint8_t i=0; i < 4; i++) {
            if (adc.AdcVals[i] <= ADC_LOW_BORDER) {              
                if (TempState[i] != ADCIN_LOW) 
                    message.send_PGN_45(127, 7); // передаем значения АЦП
                TempState[i] = ADCIN_LOW;
            } else
            if (adc.AdcVals[i] >= ADC_HIGH_BORDER) {
                if (TempState[i] != ADCIN_HIGH) 
                    message.send_PGN_45(127, 7); // передаем значения АЦП
                TempState[i] = ADCIN_HIGH;
            } else {
                TempState[i] = ADCIN_OK;
            }
        }
    }
    if ((core.getSysTick()-TimeTransPgn15) > 5000) { // ШИМ
        TimeTransPgn15 = core.getSysTick();
        message.send_PGN_15(127, 7);
    }
    if ((core.getSysTick()-TimeTransPgn44) > 5000) { // АЦП
        TimeTransPgn44 = core.getSysTick();
        message.send_PGN_44(127, 7);
    }
    if ((core.getSysTick()-TimeTransPgn45) > 5000) { // температура
        TimeTransPgn45 = core.getSysTick();
        message.send_PGN_45(127, 7);
    }
}
//-------------------------------------------------
void CanMessage::Process(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7)
{
    uint16_t PGN = AID >> 20;
    uint8_t TransType = (AID >> 3)&127;
    uint8_t TransAddr = AID & 7;
    uint32_t V32;

    uint32_t newID = (TransType<<13)+(TransAddr<<10)+(can.idType<<3)+can.idAddress;
        switch(PGN) {
        case 1: //Команды
                newID = (2<<20)+(TransType<<13)+(TransAddr<<10)+(can.idType<<3)+can.idAddress;
                if (((AD0<<8)+AD1) == 22) {//Reset CPU
                    //ToDo сделать проверку принятых байтов AA 55
                    if (AD2==0) *(__IO uint32_t*)(NVIC_VECTTAB_RAM+DATA_OFFSET) = 0x0016AA55; //переход к загрузке
                    if (AD2==1) *(__IO uint32_t*)(NVIC_VECTTAB_RAM+DATA_OFFSET) = 0x001655AA; //остаться в основной программе 
                    NVIC_SystemReset();
                } else {
                    can.SendMessage(newID, AD0,AD1,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
                }
            break;
        case 6: //запрос по PGN
           if (((AD0<<8) + AD1) == 18) {
                newID=(18<<20)+(TransType<<13)+(TransAddr<<10)+(can.idType<<3)+can.idAddress;
                can.SendMessage(newID, VER_PRODUCT_TYPE,VER_VOLTAGE,VER_PRODUCT_SUBTYPE,VER_ASSEMBLAGE_NUMBER,0xFF,VER_DAY,VER_MONTH,VER_YEAR);               
           } else if (((AD0<<8) + AD1) == 44) {
                send_PGN_44(TransType, TransAddr);  
           }
           else {
               //can.SendMessage((18<<20)+(TransType<<13)+(TransAddr<<10)+(can.idType<<3)+can.idAddress,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
           }
          break; 
       
        case 7: //работа с параметрами конфигурации
            newID=(7<<20)+(TransType<<13)+(TransAddr<<10)+(can.idType<<3)+can.idAddress;
            switch(AD0) {
                case MEMORY_0_ERASE://----------------------  0 - стереть конфигурацию
                        config.Erase();
                        can.SendMessage(newID,MEMORY_4_ANSWER,AD0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
                        break;
                case MEMORY_1_WRITE_TO_RAM://---------------  1 - запись параметра в ОЗУ
                        config.WriteToRAM((AD2<<8)+AD3,(AD4<<24)+(AD5<<16)+(AD6<<8)+AD7);
                        can.SendMessage(newID,MEMORY_4_ANSWER,AD0,AD2,AD3,AD4,AD5,AD6,AD7);
                        break;
                case MEMORY_2_WRITE_ALL_TO_FLASH://---------  2 - запись всех параметров работы во flash
                        config.WriteAllToFlash();
                        can.SendMessage(newID,MEMORY_4_ANSWER,AD0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
                        break;
                case MEMORY_3_READ://-----------------------  3 - чтение параметра по номеру
                        //чтение параметра по принятому имени (номеру)
                        V32=config.ReadFromRAM((AD2<<8)+AD3);//считали параметр
                        can.SendMessage(newID,MEMORY_4_ANSWER,AD0,AD2,AD3,V32>>24,V32>>16,V32>>8,V32);
                        break;
                default:can.SendMessage(newID,MEMORY_5_ANSWER,AD0,AD2,AD3,0xFF,0xFF,0xFF,0xFF);
            }
        break;
        case 43:
            Load.setPower(LOAD1, AD0*256 + AD1);
            Load.setPower(LOAD2, AD2*256 + AD3);
            Load.setPower(LOAD3, AD4*256 + AD5);
            send_PGN_44(TransType, TransAddr);      
        break;
    }
        
}
//=======================================================
void CanMessage::send_PGN_44(uint8_t transType, uint8_t transAddr) // ШИМ нагрузок
{
    uint16_t L1, L2, L3, L4;
    L1 = Load.getPower(LOAD1);
    L2 = Load.getPower(LOAD2);
    L3 = Load.getPower(LOAD3);
    //L4 = Load.getPower(LOAD4);
    L4 = 0;
    can.SendMessage((44<<20)+(transType<<13)+(transAddr<<10)+(can.idType<<3)+can.idAddress, L1>>8,L1, L2>>8,L2, L3>>8,L3, L4>>8,L4);
    TimeTransPgn44 = core.getSysTick();
}
//-------------------------------
void CanMessage::send_PGN_15(uint8_t transType, uint8_t transAddr) // АЦП датчиков
{
    uint16_t L1, L2, L3, L4;
    L1 = adc.AdcAverVals[chTS1];
    L2 = adc.AdcAverVals[chTS2];
    L3 = adc.AdcAverVals[chTS3];
    L4 = adc.AdcAverVals[chTS4];
    can.SendMessage((15<<20)+(transType<<13)+(transAddr<<10)+(can.idType<<3)+can.idAddress, L1>>8,L1, L2>>8,L2, L3>>8,L3, L4>>8,L4);  
    TimeTransPgn15 = core.getSysTick();
}
//-------------------------------
void CanMessage::send_PGN_45(uint8_t transType, uint8_t transAddr) // температуры датчиков
{
    int16_t T1, T2, T3, T4;
    T1 = adc.Temp1;
    T2 = adc.Temp2;
    T3 = adc.Temp3;
    T4 = adc.Temp4;
    can.SendMessage((45<<20)+(transType<<13)+(transAddr<<10)+(can.idType<<3)+can.idAddress, T1>>8,T1, T2>>8,T2, T3>>8,T3, T4>>8,T4); 
    TimeTransPgn45 = core.getSysTick();
}
//-------------------------------

