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

//nRF52 DK
#define LED1 17
#define LED2 18
#define LED3 19
#define LED4 20

#define RESTORE_SOFTDEVICE 1
#undef SOFTDEVICE_PRESENT

#define COUNT 1
#define DMA_COUNT 0x1000
#define RTC_COUNT_MAX 20

int16_t result1[DMA_COUNT] __attribute__((section (".mydata1")));
int16_t result2[DMA_COUNT] __attribute__((section (".mydata2")));

#define SAADC_LOW_POWER

#ifdef SAADC_LOW_POWER
#include "saadc_lowpower.c"
#else
#include "saadc.c"
#endif


int main(void)
{
    nrf_gpio_cfg_output(LED2);
	nrf_gpio_pin_write(LED2,1);

	nrf_gpio_cfg_output(LED3);
	nrf_gpio_pin_write(LED3,1);

	
    //Configure 
    NRF_SAADC->CH[0].CONFIG = (
        SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        2 << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_GAIN_Gain2  << SAADC_CH_CONFIG_GAIN_Pos |
        SAADC_CH_CONFIG_RESP_Pullup << SAADC_CH_CONFIG_RESN_Pos |
        SAADC_CH_CONFIG_RESP_Pullup << SAADC_CH_CONFIG_RESP_Pos );

	//Setup AIN4 and AIN5 as inputs
    NRF_SAADC->CH[0].PSELP = 0x5;
    NRF_SAADC->CH[0].PSELN = 0x6;

    //Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;
    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over2x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;

	    //Use external timing
	NRF_SAADC->SAMPLERATE = 0;


	
	
#ifndef SAADC_LOW_POWER
	ppi_init();
#endif
	saadc_init();
	clock_init();


	NVIC_SetPriority(SAADC_IRQn, 0);
    NVIC_ClearPendingIRQ(SAADC_IRQn);
    NVIC_EnableIRQ(SAADC_IRQn);




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
