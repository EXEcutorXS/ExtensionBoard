/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/
#ifndef __CAN_MASSAGE_H
#define __CAN_MASSAGE_H
/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"
//------------------------------------------------------------------------------
#define MEMORY_0_ERASE              0                                //стереть сектор памяти хранения данных
#define MEMORY_1_WRITE_TO_RAM       1                                //запись данных 
#define MEMORY_2_WRITE_ALL_TO_FLASH 2                                //завершение записи данных 
#define MEMORY_3_READ               3                                //чтение данных
#define MEMORY_4_ANSWER             4                                //ответ на запрос
#define MEMORY_5_ANSWER             5                                //ответ на запрос - невозможно выполнить команду

#define ADCIN_NUMB          4
#define ADC_LOW_BORDER      200
#define ADC_HIGH_BORDER     4000

#define ADCIN_OK        0
#define ADCIN_LOW       1
#define ADCIN_HIGH      2
//---------------------------------------------------------------------

class CanMessage
{
    public:
        void initialize(void);
        void handler(void);
		void send_PGN_15(uint8_t transType, uint8_t transAddr);    //RAW ADC
		void send_PGN_44(uint8_t transType, uint8_t transAddr);    //PWM Status
        void send_PGN_45(uint8_t transType, uint8_t transAddr);    //Temperatures 
//        void Transmit(uint8_t* buf, uint8_t len);
//        uint8_t getMessage(uint8_t* buf);
//        void printLog(char *str);
//        void handleError(char *str);
    private:
        void Process(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7);

    uint32_t TimeTransPgn15, TimeTransPgn44, TimeTransPgn45;
    uint8_t TempState[4];
};
extern CanMessage message;
//--------------------------------------------
#endif /* __CAN_MASSAGE_H */
//===============================
