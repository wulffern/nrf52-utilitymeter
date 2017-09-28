/*********************************************************************
 *        Copyright (c) 2017 Carsten Wulff Software, Norway
 * *******************************************************************
 * Created       : wulff at 2017-8-26
 * *******************************************************************
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/


#ifndef HAL_UTILITY_CW__
#define HAL_UTILITY_CW__

#include <stdint.h>
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
#define RTC_PRESCALE         (1)        //Slow down RTC clock
#define RTC_COUNT_PER_MINUTE (32768)    //RTC frequency
#define TICKS_TO_AVERAGE     (1024)     //How many ticks to average
#define BLINKS_PER_KWH       (10000)    //Setting on utility meter,
                                        //change to what yours say

enum sensor_state{
    SAADC_CAPTURE_INIT,
	SAADC_CAPTURE,    
	POWER_DATA_READY,
	ADVERTIZE
};

typedef enum sensor_state sensor_state_t;


void SAADC_IRQHandler(void);
void RTC2_IRQHandler(void);

void hal_utility_saadc_init();
void hal_utility_clock_init();
void hal_utility_rtc_init();
void hal_utility_init();
void hal_utility_handler(float wh);
void send_one_packet(uint8_t channel_index);
void hal_utility_state_machine();



#endif
