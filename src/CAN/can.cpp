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
#include "version.h"

//==================================================================

Can can;
//-------------------------------------------------------------------
void Can::initialize(void)
{
    uint32_t idFilter;
    uint32_t idMAsk;
    idType = VER_PRODUCT_TYPE; // VER_PRODUCT_TYPE;                           //тип устройства                                              //адрес устройства
    // Enable the Clock
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_PMU);
    
    can_deinit(CAN0);
    
    // Configure the CAN pin
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9); // CAN TX
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8); // |CAN RX
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13); // |CAN RX
    gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP, ENABLE);
    
	gpio_bit_reset(GPIOC,GPIO_PIN_13);
	
    can_parameter_struct canInitStruct;
    can_struct_para_init(CAN_INIT_STRUCT, &canInitStruct);
    //Configure CAN
    canInitStruct.working_mode = CAN_NORMAL_MODE;                                /*!< CAN working mode */ // CAN_NORMAL_MODE
    canInitStruct.resync_jump_width = CAN_BT_SJW_1TQ;                            /*!< CAN resynchronization jump width */
    canInitStruct.time_segment_1 = CAN_BT_BS1_5TQ;                               /*!< time segment 1 */
    canInitStruct.time_segment_2 = CAN_BT_BS2_2TQ;                               /*!< time segment 2 */
    canInitStruct.time_triggered = DISABLE;                                      /*!< time triggered communication mode */
    canInitStruct.auto_bus_off_recovery = ENABLE;                               /*!< automatic bus-off recovery */
    canInitStruct.auto_wake_up = ENABLE;                                        /*!< automatic wake-up mode */
    canInitStruct.no_auto_retrans = ENABLE;                                      /*!< automatic retransmission mode disable */
    canInitStruct.rec_fifo_overwrite = DISABLE;                                  /*!< receive FIFO overwrite mode */
    canInitStruct.trans_fifo_order = DISABLE;                                    /*!< transmit FIFO order */
    canInitStruct.prescaler = SystemCoreClock/(canInitStruct.time_segment_1+canInitStruct.time_segment_2+3)/CAN_SPEED;                          /*!< baudrate prescaler */
    
    can_init(CAN0, &canInitStruct);
    
    can_filter_parameter_struct canfilterSetup;
    can_struct_para_init(CAN_FILTER_STRUCT, &canfilterSetup);
   
    canfilterSetup.filter_number = 0;
    canfilterSetup.filter_fifo_number = CAN_FIFO1;
    canfilterSetup.filter_bits = CAN_FILTERBITS_32BIT;
    canfilterSetup.filter_mode = CAN_FILTERMODE_MASK;
    idMAsk =   (((0<<20)+(127<<13)+(7<<10)+(0<<3)+(0<<0)) << 3) + 4; // 4 - extend id bit // 0x0000DE3F6 & 0x006F63F6
    idFilter = (((0<<20)+(idType<<13)+(idAddress<<10)+(0<<3)+(0<<0)) << 3) + 4;
    canfilterSetup.filter_list_high = idFilter >> 16;
    canfilterSetup.filter_list_low = idFilter & 0x0000ffff;
    canfilterSetup.filter_mask_high = idFilter >> 16;
    canfilterSetup.filter_mask_low = idFilter & 0x0000ffff;
    canfilterSetup.filter_enable = ENABLE;
    can_filter_init(&canfilterSetup);
   
    canfilterSetup.filter_number = 1; // 
    canfilterSetup.filter_fifo_number = CAN_FIFO1;
    canfilterSetup.filter_bits = CAN_FILTERBITS_32BIT;
    canfilterSetup.filter_mode = CAN_FILTERMODE_MASK;
    idMAsk =   (((0<<20)+(127<<13)+(7<<10)+(0<<3)+(0<<0)) << 3) + 4;
    idFilter = (((0<<20)+(127<<13)+(7<<10)+(0<<3)+(0<<0)) << 3) + 4;
    canfilterSetup.filter_list_high = idFilter >> 16;
    canfilterSetup.filter_list_low = idFilter & 0x0000ffff;
    canfilterSetup.filter_mask_high = idMAsk >> 16;
    canfilterSetup.filter_mask_low = idMAsk & 0x0000ffff;
    canfilterSetup.filter_enable = ENABLE;
    can_filter_init(&canfilterSetup);
          
    can_interrupt_enable(CAN0, CAN_INT_RFNE1); 
    nvic_irq_enable(CAN0_RX1_IRQn, 0, 1);
    
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ, GPIO_PIN_5);// 
    gpio_bit_reset(GPIOA, GPIO_PIN_5); // standby disable
    noConnectionS = 0;
}
//-------------------------------------------------------------------
void Can::ChipStandByEnable(void)
{
    gpio_bit_set(GPIOA, GPIO_PIN_5); // standby enable
    
    gpio_init(GPIOB, GPIO_MODE_IPD ,GPIO_OSPEED_50MHZ, GPIO_PIN_8);// 
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_8); // can_rx PB8 as exti8 sourse
    exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_8);
    exti_interrupt_enable(EXTI_8);
    nvic_irq_enable(EXTI5_9_IRQn, 1, 1); 
}
void Can::ChipStandByDisable(void)
{
    exti_interrupt_disable(EXTI_8);
    nvic_irq_disable(EXTI5_9_IRQn);
    gpio_bit_reset(GPIOA, GPIO_PIN_5); // standby disable
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8); // |CAN RX
}
//---------------------------------------
void Can::Stop(void)
{
    uint32_t time;
//    can_transmit_state_enum status;
    for(uint8_t i = 0; i < 3; i++) {
        can_transmission_stop(CAN0, i);        
    }
    time = core.getSysTick() + 10;
    while (core.getSysTick() < time) {
    }
    can_interrupt_disable(CAN0, CAN_INT_RFNE1); 
    nvic_irq_disable(CAN0_RX1_IRQn);
    can_deinit(CAN0);
}
//-----------------------------------
void Can::handler(void)
{
    static uint32_t TimeoutPeriod;
    static uint32_t noConnection_Tick;
    can_transmit_state_enum status;
    uint8_t isTrans = 0;

    if (core.getSysTick() >= TimeoutPeriod) {
        TimeoutPeriod = core.getSysTick() + 10;
        for(uint8_t i = 0; i < 3; i++) {
            if (transTime[i] > 0) {
                transTime[i]--;
                isTrans = 1;
            }
            if (transTime[i] == 1) { 
                status = can_transmit_states(CAN0, i);
                if (status != CAN_TRANSMIT_OK) can_transmission_stop(CAN0, i);   
            }
        }
        can.isTrans = isTrans;
        if (can_error_get(CAN0) != CAN_ERROR_NONE) {
            
            can_flag_clear(CAN0, CAN_FLAG_MAL0); //
            can_flag_clear(CAN0, CAN_FLAG_MTE0); //
            can_flag_clear(CAN0, CAN_FLAG_SLPIF); //
            can_flag_clear(CAN0, CAN_FLAG_WUIF); //
            can_flag_clear(CAN0, CAN_FLAG_ERRIF); //
        }
    }
    if ((core.getSysTick() - noConnection_Tick) >= 1000) {
        noConnection_Tick = core.getSysTick();
        noConnectionS++;
    }

}
//--------------------------------
void Can::SendMessage(uint32_t AID, uint8_t AD0, uint8_t AD1, uint8_t AD2, uint8_t AD3, uint8_t AD4, uint8_t AD5, uint8_t AD6, uint8_t AD7)
{
    uint8_t TransmitMailBox=0;
    
    //can_transmit_state_enum status;
    
    Can_TxMessage.tx_sfid = 0;                                    /*!< standard format frame identifier */
    Can_TxMessage.tx_efid = AID;                                  /*!< extended format frame identifier */
    Can_TxMessage.tx_ff = CAN_FF_EXTENDED;                        /*!< format of frame, standard or extended format */
    Can_TxMessage.tx_ft = CAN_FT_DATA;                            /*!< type of frame, data or remote */
    Can_TxMessage.tx_dlen = 8;                                    /*!< data length */
    Can_TxMessage.tx_data[0] = AD0;
    Can_TxMessage.tx_data[1] = AD1;   
    Can_TxMessage.tx_data[2] = AD2;  
    Can_TxMessage.tx_data[3] = AD3;  
    Can_TxMessage.tx_data[4] = AD4;  
    Can_TxMessage.tx_data[5] = AD5;  
    Can_TxMessage.tx_data[6] = AD6;  
    Can_TxMessage.tx_data[7] = AD7;  

    TransmitMailBox = can_message_transmit(CAN0, &Can_TxMessage);
    if (TransmitMailBox < 3) transTime[TransmitMailBox] = 10;
    /*
    uint32_t i=0;
    status = can_transmit_states(CAN0, TransmitMailBox);
    while(status != CAN_TRANSMIT_OK) {
        status = can_transmit_states(CAN0, TransmitMailBox);
        i++;
        if (i > 60000) {
            can_transmission_stop(CAN0, TransmitMailBox);
            break;
        }
    } 
    */
    
}
//-------------------------------------------------------------------
//Сохранить сообщение RxMessage в буфере MsgBuf
void Can::SaveMessage(void)
{
    for(uint8_t i=0; i<MSG_BUF_SIZE; i++) {
        if (!MsgBuf[i].Received_f) {
            MsgBuf[i].Received_f = 1;
            noConnectionS = 0;
            uint8_t* disAdr = (uint8_t*)&MsgBuf[i].Msg;
            uint8_t* srcAdr = (uint8_t*)&Can_RxMessage;
            for (uint8_t i=0; i<sizeof(can_receive_message_struct); i++) {
                *disAdr++ = *srcAdr++; 
            }
            return;
        }
    }
}
//-------------------------------------------------------------------

//Прерывание по приему CAN
void Can::RX0_Interrupt(void)
{
    if (can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL0) == SET) {                  // Прерывание получения пакета в буфер FIFO 0
                                                                     // Флаг сбрасывается автоматически после прочтения последнего сообщения              
        can_message_receive(CAN0, CAN_FIFO0, &Can_RxMessage);        // Получим сообщение
        SaveMessage();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL0); // receive FIFO0 full interrupt flag
        
    }

}
void Can::RX1_Interrupt(void)
{
    if (can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL1) == SET) {                  // Прерывание получения пакета в буфер FIFO 1
                                                                     // Флаг сбрасывается автоматически после прочтения последнего сообщения
        can_message_receive(CAN0, CAN_FIFO1, &Can_RxMessage);        // Получим сообщение
        SaveMessage();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL1); // receive FIFO1 full interrupt flag

    }
}
//==================================
extern "C" void CAN0_RX1_IRQHandler(void)
{
    can.RX1_Interrupt();
}   
             
//-----------------------------------------------------------------
extern "C" void EXTI5_9_IRQHandler(void)
{ // EXTI_8 PB8 CANRX
    exti_interrupt_flag_clear(EXTI_8);
    //Can_ChipStandByDisable();
}
