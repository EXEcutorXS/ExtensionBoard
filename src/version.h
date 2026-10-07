/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VERSION_H
#define __VERSION_H

    #define MAIN_PROGRAM_START_ADDRESS  0x08008000

    #define VER_PRODUCT_TYPE        37
    #define VER_VOLTAGE             0
    #define VER_PRODUCT_SUBTYPE     3
    #define VER_ASSEMBLAGE_NUMBER   4
                                       //дата создания версии
    #define VER_DAY                6  //день
    #define VER_MONTH              10  //месяц
    #define VER_YEAR               26  //год (последние две цифры)
    
    #define DATA_OFFSET                 0xBFFC

#endif /* __VERSION_H */


/*
37.0.3.4
Software CAN filter added
clamping input value to 1000
1st channel switched to software PWM 25 kHz
Added second hardware PWM to PB5, doubling PA7
Driver control added. Disabled when all channels are off
WFI__ added to the end of main cycle
*/
