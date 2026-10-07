/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

/* Includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/* Defines ------------------------------------------------------------------*/
#define NUMB_OF_DMA_CYCLE   16
#define NUMB_OF_CHAN   7                   // количество измеряемых каналов
#define LEN_ARRAY_DMA   NUMB_OF_CHAN*NUMB_OF_DMA_CYCLE      // длина массива для DMA

//------------------------------------------------------------------------------------
// таблица сопротивлений MF52A103F3950 Rt (номинал 10 КОм)  от -55 до 115 *С с шагом 5*
//                           -55    -50      -45     -40     -35     -30    -25     -20     -15     -10     -5      0       5       10      15      20     25     30      35
const float NTC_table[] = { 500.05, 400.81, 340.44, 280.66, 223.84, 159.78,116.02, 87.261, 67.214, 52.35, 40.929, 32.049, 25.149, 19.856, 15.677, 12.485, 10.0,  8.06,   6.533,
//                              40      45      50      55      60     65      70      75      80      85     90      95      100     105     110     115    120    125    130
                              5.325,   4.361,  3.59,   2.968,  2.467,  2.06,  1.728,  1.457,  1.235,  1.052, 0.899,  0.772,  0.666,  0.575,  0.499,  0.433, 0.377, 0.329, 0.286
};
//-----------------------------------------------------------------------------------
enum
{
    chTS3,                      // Температура IN_AI3
    chTS4,                      // Температура IN_AI4
    chTS1,                      // Температура IN_AI2  
    chTS2,                      // Температура IN_AI1  
    chVolt,                     // напряжение питания   
    chTcpu,                     // Температура процессора
    chURef,                     // Опорное напряжение для АЦП
};

/* Classes ------------------------------------------------------------------*/
class ADCmodule
{
   
    public:
        ADCmodule(void);

        void initialize(void);
        void handler(void);
        void Averaging(void); // осреднение значений, накопленных в буфере DMA

        uint16_t AdcVals[NUMB_OF_CHAN];          // буфер для каналов
        uint16_t AdcAverVals[NUMB_OF_CHAN];          // буфер для средних значений каналов
        uint16_t bufDMA[LEN_ARRAY_DMA];             // Буфер для DMA 
        int16_t Temp1; // 0.1 C
        int16_t Temp2; // 0.1 C
        int16_t Temp3; // 0.1 C
        int16_t Temp4; // 0.1 C
        uint16_t Voltage;  // 0.1 V
        int16_t Tcpu;  // 0.1 C
        uint16_t Uref; // mV, напряжение питания АЦП (по Vrefint)
        bool ready_f;

    private:
        int16_t calcNTC(uint16_t adcVal, uint16_t ref);
        uint16_t calcVoltage(uint16_t adcVal, uint16_t refin);
        
        uint32_t SumADCch[NUMB_OF_CHAN];     // здесь суммируются значения ацп
        uint16_t cntSum;      
        uint8_t numbMeas;
        
};

extern ADCmodule adc;

#endif /* __ADC_H */
