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
#include "ADCmodule.h"
#include "core.h"
#include "gd32f30x_dma.h"

ADCmodule adc;
//-----------------------------------------------------
ADCmodule::ADCmodule(void)
{

}
//-----------------------------------------------------
void ADCmodule::initialize(void)                                      // ИНИЦИАЛИЗАЦИЯ ПЕРИФЕРИИ
{
    for (uint8_t i = 0; i < NUMB_OF_CHAN; i++) {
        SumADCch[i] = 0;     // обнуляем массив
        AdcVals[i] = 0;      // буфер для каналов
        AdcAverVals[i] = 0;  // буфер для средних значений каналов
    }
    cntSum = 0;
    ready_f = false;
    numbMeas = 0;

    // настройка портов аналогового ввода
    rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV2);

    adc_disable(ADC0);
    adc_deinit(ADC0);
    dma_deinit(DMA0, DMA_CH0);

    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1); // MCU_AI3
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_2); // MCU_AI4
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_3); // MCU_AI2
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_4); // MCU_AI1
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_6); // Voltage
    
	//Enabling pullup for NTC sensors
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_3); //sens 1K
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7); //sens 10k
	gpio_bit_reset(GPIOB, GPIO_PIN_7);//sens 10k for NTC
	
    // Настраиваем АЦП
    adc_resolution_config(ADC0 , ADC_RESOLUTION_12B);
    adc_mode_config(ADC_MODE_FREE);
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);        // !
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, NUMB_OF_CHAN);

    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5); // ADC_SAMPLETIME_71POINT5
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_2, ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_3, ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_4, ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_6, ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 5, ADC_CHANNEL_16, ADC_SAMPLETIME_239POINT5);
    adc_regular_channel_config(ADC0, 6, ADC_CHANNEL_17, ADC_SAMPLETIME_239POINT5);

    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);        // !
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);        // !
    
    adc_tempsensor_vrefint_enable(); // Включаем внутренний датчик температуры процессора // Включаем источник опорного напряжения

//    adc_interrupt_disable(ADC0 , ADC_INT_EOC);
//    adc_interrupt_disable(ADC0 , ADC_INT_EOIC);
    
    adc_enable(ADC0);                                                           // Включаем АЦП        // !
    adc_calibration_enable(ADC0);
    //adc_dma_mode_enable(ADC0);

//    nvic_irq_enable(ADC0_1_IRQn, 0, 4);
    
    dma_periph_address_config(DMA0, DMA_CH0, (uint32_t)(&ADC_RDATA(ADC0)));
    dma_memory_address_config(DMA0, DMA_CH0, (uint32_t)this->bufDMA);
    dma_circulation_enable(DMA0, DMA_CH0);
    dma_transfer_number_config(DMA0, DMA_CH0, LEN_ARRAY_DMA);
    dma_priority_config(DMA0, DMA_CH0, DMA_PRIORITY_HIGH);
    dma_memory_width_config(DMA0, DMA_CH0, DMA_MEMORY_WIDTH_16BIT); // DMA_MEMORY_WIDTH_16BIT
    dma_periph_width_config (DMA0, DMA_CH0, DMA_PERIPHERAL_WIDTH_16BIT);    // DMA_PERIPHERAL_WIDTH_16BIT
    dma_memory_increase_enable(DMA0, DMA_CH0);
    dma_periph_increase_disable(DMA0, DMA_CH0);
    dma_transfer_direction_config(DMA0, DMA_CH0, DMA_PERIPHERAL_TO_MEMORY);
    
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    dma_channel_enable(DMA0, DMA_CH0);
    
    adc_enable(ADC0);                                                           // Включаем АЦП        // !
//    adc_calibration_enable(ADC0);
    adc_dma_mode_enable(ADC0);
    adc_software_trigger_enable(ADC0 , ADC_REGULAR_CHANNEL);
    nvic_irq_enable(DMA0_Channel0_IRQn, 1, 3);
}
//-----------------------------------------------------
void ADCmodule::disable(void)
{
    adc_disable(ADC0);
    rcu_periph_clock_disable(RCU_ADC0);
    rcu_periph_clock_disable(RCU_DMA0);
}
//-----------------------------------------------------
void ADCmodule::enable(void)
{
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_6);
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_3); //sens 1K
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7); //sens 10k
	
	gpio_bit_reset(GPIOB, GPIO_PIN_7);//sens 10k for NTC
  
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    dma_channel_enable(DMA0, DMA_CH0);
    adc_enable(ADC0);                                                           // Включаем АЦП        // !

    adc_calibration_enable(ADC0);
    adc_dma_mode_enable(ADC0);

    nvic_irq_enable(DMA0_Channel0_IRQn, 1, 3);
//    NVIC_SetPriority(DMA0_Channel0_IRQn, 2);                                      // разрешаем прерывание
//    NVIC_EnableIRQ(DMA0_Channel0_IRQn);
    
}
//-----------------------------------------------------
void ADCmodule::handler(void)                                                 // ФУНКИЯ ПЕРИОДИЧЕСКОГО ОПРОСА
{
    static uint32_t lastTickCalc=0;                                             // время на расчет измеряемых величин

    if (core.getSysTick()-lastTickCalc > 1000) {                                   // раз в 250 мсек расчитываем аналоговые величины
        lastTickCalc=core.getSysTick();                                            // сохраняем настоящее время
        if (cntSum == 0) return;
        __disable_irq();       
        for (uint8_t ch = 0; ch < NUMB_OF_CHAN; ch++) {
            AdcAverVals[ch] = SumADCch[ch] / cntSum;
            SumADCch[ch] = 0;
        } 
        cntSum = 0;
        __enable_irq();
        
        static const uint16_t Ion = 3300; // mv
        
        uint16_t val = (uint32_t)AdcAverVals[chTcpu] * Ion/4095;
        Tcpu = (int8_t)(((float)(1450 - val)/4.1 + 25.0));//  
                
        // тип датчика - NTC
        Temp1 = calcNTC(AdcAverVals[chTS1], Ion);
        Temp2 = calcNTC(AdcAverVals[chTS2], Ion);
        Temp3 = calcNTC(AdcAverVals[chTS3], Ion);
        Temp4 = calcNTC(AdcAverVals[chTS4], Ion);
        Voltage = calcVoltage(AdcAverVals[chVolt], Ion); 
        Uref = (uint32_t)AdcAverVals[chTcpu] * Ion/4095;
        
        if (!ready_f) {
            numbMeas++;
            if (numbMeas > 1) ready_f = true; 
        }

    }

}
//-----------------------------------------------------
    // Расчёт напряжения питания Вольт
uint16_t ADCmodule::calcVoltage(uint16_t adcVal, uint16_t refin)                                 // РАСЧЕТ Напряжения питания
{
    uint16_t mv = (uint32_t)adcVal*refin*11/4095/100; // 0.1 В
    return mv;
}
//------------------------------------------------------
//float ADCmodule::calcKTY(uint16_t adcVal, uint16_t ref)                                               // РАСЧЕТ ТЕМПЕРАТУРЫ ДТ и ДП
//{
//    float u = (float)adcVal*ref/4095.0/1000.0;
//    
//    return (float)(u*u*u*36.72-u*u*149.27+u*333-310.15);                                 // квадратичная апроксимация данных датчика из таблицы
//}
////------------------------------------------------------
//float ADCmodule::calcLM(uint16_t adcVal, uint16_t ref)                                               // РАСЧЕТ ТЕМПЕРАТУРЫ ДТ и ДП
//{
//    float u = (float)adcVal*ref/4095.0/1000.0;
//    
//    return u*2*100 - 273.0;                                                             // линейная апроксимация данных датчика
//}
//===============================================================
// расчет температуры по NTC резистору MF52A103F3950  
// adcVal - значение АЦП; refin - опорное напряжение в мВ
// результат расчета град С *10 ( т.е. с шагом 0,1 С )
/*
                 Ref +3.3V
                  _
                 | | R1 - 10 KOm
                 |_|
                   |_____ U ADC 
                  _|
                 | | R2 - 300 Om
                 |_|
                  _|                  
                 | | NTC
                 |_|

*/
int16_t ADCmodule::calcNTC(uint16_t adcVal, uint16_t refin)                                 // РАСЧЕТ ТЕМПЕРАТУРЫ ДТ NTC ( в 0,1 С )
{
    float R1 = 10.0; // 10 КОм
    float R2 = 0.3;  // 300 Ом
    float ref = (float)refin/1000.0;
    float U = (float)adcVal*ref/4095.0; // напряжение на входе АЦП
    float R = (ref*R1/(ref-U))-R1-R2;
    
    float temp = -127.0; // обрыв датчика 
    if (R < 600.0) {
        if (R > NTC_table[0]) { // слишком низкая температура 
            temp = -60.0;
        } else {
            uint8_t i = 0;
            uint8_t lastIndx = (sizeof(NTC_table)/sizeof(NTC_table[0]))-1; 
            float a = NTC_table[lastIndx-1];
            float b = NTC_table[lastIndx];
            for (i = 0; i < (lastIndx-1); i++) {
                a = NTC_table[i];
                b = NTC_table[i+1];
                if ( (R <= a) && (R >= b) ) {
                    break;
                }
            }
            float dT = (a-R)*5 / (a-b);
            temp = -55.0 + i*5 + dT;
        }
    } 
    return (int16_t)(temp*10);                 
}
//-----------------------------------------------------
// В прерывании высчитывается среднее значение по массиву DMA и 
// полученное значение суммируется в SumADCch[ch] для дальнейшего осреднения
// в основном цикле за больший период
void ADCmodule::Averaging(void)
{
    uint32_t Sum;
    
    for (uint8_t ch = 0; ch < NUMB_OF_CHAN; ch++) {
        Sum = 0;
        for(uint8_t i = 0; i < NUMB_OF_DMA_CYCLE; i++) {
            Sum += bufDMA[ch + i*NUMB_OF_CHAN];          
        } 
        AdcVals[ch] = Sum / NUMB_OF_DMA_CYCLE;
        SumADCch[ch] += AdcVals[ch];
    } 
    cntSum++;
}
//-----------------------------------------------------
// расчет температуры по NTC резистору B57332V5103F360 на плате
    
//float ADCmodule::NTCcalculate(uint16_t adcVal, float refin)
//{
//    float Rnom = 10.0; // 10000.0 ОМ 
//    float ref = (float)refin/1000.0;
//    float u = (float)adcVal*ref/4095.0;
//    float div = ref-u;
//    if (div == 0) div = 0.01;
//    float R = u*Rnom/(div); // 10 - 10КОм // R = U*R1/(ION-U)
//    float temp = -55.0;
//    uint8_t i = 0;
//    uint8_t lastIndx = (sizeof(NTC_table)/sizeof(NTC_table[0]))-1; 
//    float a = NTC_table[lastIndx-1];
//    float b = NTC_table[lastIndx];
//    
//    if (R > NTC_table[0]) { // слишком низкая температура или обрыв датчика
//        temp = -60.0;
//    } else {
//        for (i = 0; i < (lastIndx-1); i++) {
//            a = NTC_table[i];
//            b = NTC_table[i+1];
//            if ( (R <= a) && (R >= b) ) {
//                break;
//            }
//        }
//        float dT = (a-R)*5 / (a-b);
//        temp = -55.0 + i*5 + dT;
//    }
//    return temp;
//}
//-----------------------------------------------------
extern "C" void DMA0_Channel0_IRQHandler(void)                                             // ОБРАБОТКА ПРЕРЫВАНИЯ от DMA
{
    if (dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FLAG_FTF)){
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
        adc.Averaging();
    }
}
//=======================================================================

