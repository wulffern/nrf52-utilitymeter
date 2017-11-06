//====================================================================
//        Copyright (c) 2017 Carsten Wulff Software, Norway 
// ===================================================================
// Created       : wulff at 2017-08-26
// ===================================================================
//  The MIT License (MIT)
// 
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
// 
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
// 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//  
//====================================================================



#ifndef HAL_UTILITY_CW__
#define HAL_UTILITY_CW__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "math.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "hal_radio.h"

#ifndef LED1
#define LED1 17
#endif

#ifndef LED2
#define LED2 18
#endif

#ifndef LED3
#define LED3 19
#endif


#ifndef LED4
#define LED4 20
#endif


#define DBG_STATES

#define DMA_COUNT            (0x1000)   //RAM buffer to store data
#define RTC_COUNT_MAX        (64)       //Compare value for RTC, sets current
#define RTC_PRESCALE         (0)        //Slow down RTC clock
#define RTC_COUNT_PER_SECOND (32768)    //RTC frequency
#define TICKS_TO_AVERAGE     (2048)     //How many ticks to average

#define BLINKS_PER_KWH       (10000)    //Setting on utility meter,
                                        //change to what yours say (blinks/kwh)

enum sensor_state{
    SAADC_CAPTURE_INIT,
	SAADC_CAPTURE,    
	POWER_DATA_READY,
	ADVERTIZE
};

typedef enum sensor_state sensor_state_t;


void SAADC_IRQHandler(void);
void RADIO_IRQHandler(void);
void RTC2_IRQHandler(void);

void hal_utility_saadc_init();
void hal_utility_clock_init();
void hal_utility_rtc_init();
void hal_utility_init();
void send_one_packet(uint8_t channel_index);
void hal_utility_state_machine();



#endif
