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

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "math.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"


//Average current consumption of 6uA with these settings
#define COUNT 1
#define DMA_COUNT 2000
#define RTC_COUNT_MAX 100


//nRF52 DK
#define LED1 17
#define LED2 18
#define LED3 19
#define LED4 20

#define RESTORE_SOFTDEVICE 1
#undef SOFTDEVICE_PRESENT

int16_t result1[DMA_COUNT] __attribute__((section (".mydata1")));
int16_t result2[DMA_COUNT] __attribute__((section (".mydata2")));
int16_t result;

int saadc_tgl = 0;
int ind = 0;
int swindex = 0;

void SAADC_IRQHandler(void)
{
	volatile uint32_t dummy;
	if(NRF_SAADC->EVENTS_END == 1){

		//Reset index
		if(ind >= DMA_COUNT){
			ind  = 0;
			swindex = !swindex;
		}

		//Swich buffers depending on swindex state
		if(swindex == 0){
			nrf_gpio_pin_write(LED2,1);
			result1[ind] = result;
		}else{
			nrf_gpio_pin_write(LED2,0);
			result2[ind] = result;
		}

		ind++;
		NRF_SAADC->EVENTS_END = 0;

		// Read back event register to ensure we have cleared it before exiting IRQ handler.
		dummy = NRF_RTC1->EVENTS_END;
		dummy;

	}

}

void RTC1_IRQHandler(void)
{
  volatile uint32_t dummy;
  if (NRF_RTC1->EVENTS_COMPARE[0] == 1)
  {
    NRF_RTC1->EVENTS_COMPARE[0] = 0;

    // Increment compare value with to set the beat
    NRF_RTC1->CC[0] = NRF_RTC1->COUNTER + RTC_COUNT_MAX;

	//Do a sample
	NRF_SAADC->TASKS_START = 1;
	while (NRF_SAADC->EVENTS_STARTED == 0);
	NRF_SAADC->EVENTS_STARTED = 0;
	NRF_SAADC->TASKS_SAMPLE = 1;
	while (NRF_SAADC->EVENTS_END == 0);
	NRF_SAADC->TASKS_STOP = 1;
	while(NRF_SAADC->EVENTS_STOPPED == 0);
	NRF_SAADC->EVENTS_STOPPED = 0;


    // Read back event register to ensure we have cleared it before exiting IRQ handler.
    dummy = NRF_RTC1->EVENTS_COMPARE[0];
    dummy;
  }
}


void saadc_init(){

    //Configure 
    NRF_SAADC->CH[0].CONFIG = (
        SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        2 << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_GAIN_Gain1_5  << SAADC_CH_CONFIG_GAIN_Pos |
        SAADC_CH_CONFIG_RESP_VDD1_2 << SAADC_CH_CONFIG_RESN_Pos |
        SAADC_CH_CONFIG_RESP_Pulldown << SAADC_CH_CONFIG_RESP_Pos );

	//Setup AIN4 and AIN5 as inputs
    NRF_SAADC->CH[0].PSELP = 0x5;
    NRF_SAADC->CH[0].PSELN = 0x6;

    //Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;
    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over4x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;

    //Setup DMA buffer
    NRF_SAADC->RESULT.MAXCNT = 1;
    NRF_SAADC->RESULT.PTR = (uint32_t)&result;
	
    //Use external timing
	NRF_SAADC->SAMPLERATE = 0;
	
	// Enable SAADC
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

	//Enable END event only
	NRF_SAADC->INTEN = ( SAADC_INTEN_END_Enabled << SAADC_INTEN_END_Pos);
}


void clock_init(){

	//Start LF clock
	NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while((NRF_CLOCK->EVENTS_LFCLKSTARTED == 0));
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;


	//Setup and start RTC
    NRF_RTC1->PRESCALER = 1;
	NRF_RTC1->CC[0] = RTC_COUNT_MAX;
	NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	NVIC_EnableIRQ(RTC1_IRQn);	
	NRF_RTC1->TASKS_START = 1;	
}


int main(void)
{
    nrf_gpio_cfg_output(LED2);
	nrf_gpio_pin_write(LED2,1);

	clock_init();

    NVIC_SetPriority(SAADC_IRQn, 0);
    NVIC_ClearPendingIRQ(SAADC_IRQn);
    NVIC_EnableIRQ(SAADC_IRQn);

	saadc_init();

    // Enter main loop.
    for (;;)
    {
		__SEV();
        __WFE();
		__WFE();

    }
}


/**
 * @}
 */
