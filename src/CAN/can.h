/******************************************************************************
* ООО DD-inform
* Самара
*
* Программисты: Батырев Е.В.
*
* 14.09.2023
* Описание:
*******************************************************************************/
#ifndef __CAN_H
#define __CAN_H
/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

#define MSG_BUF_SIZE 10                                      //размер приемного буфера. Не более 255!!! иначе надо везде менять тип
#define CAN_SPEED   250000
//---------------------------------
typedef struct {
    uint8_t Received_f;
    can_receive_message_struct Msg;
} TReceivedMsg;

class Can
{
    public:
        void initialize(void);
        void handler(void);
        void RX1_Interrupt(void);
        void SendMessage(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7);

        uint8_t idType;                                              //Тип устройства
        uint8_t idAddress;                                           //Адрес устройства
        uint8_t noConnectionS;                                       //  если нет связи

        can_trasnmit_message_struct Can_TxMessage;    
        can_receive_message_struct Can_RxMessage;
        TReceivedMsg MsgBuf[MSG_BUF_SIZE];                           //Буфер принимаемых сообщений
        uint8_t transTime[3]; // время на отправку пакета
            
    private:
        
        void SaveMessage(void);                                      //Сохранить сообщение RxMessage в буфере MsgBuf

};
extern Can can;
//=========================================================
#endif /* __FLASH_H */
