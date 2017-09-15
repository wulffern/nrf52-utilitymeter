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

int16_t result;

int saadc_tgl = 0;
int ind = 0;
int swindex = 0;

void SAADC_IRQHandler(void)
{
    volatile uint32_t dummy;
    if(NRF_SAADC->EVENTS_STARTED == 1){
        if(swindex ){
            swindex = 0;
            NRF_SAADC->RESULT.PTR = (uint32_t)&result1;
        }else{

            NRF_SAADC->RESULT.PTR = (uint32_t)&result2;
            swindex = 1;
        }

//      nrf_gpio_pin_write(LED2,swindex);


        NRF_SAADC->EVENTS_STARTED = 0;

        // Read back event register to ensure we have cleared it before exiting IRQ handler.
        dummy = NRF_SAADC->EVENTS_STARTED;
        dummy;


    }

}

int toggle = 0;
int ind_led = 0;
void RTC1_IRQHandler(void)
{
    volatile uint32_t dummy;
    if (NRF_RTC1->EVENTS_COMPARE[0] == 1)
    {
        NRF_RTC1->EVENTS_COMPARE[0] = 0;


        if(ind_led == 10){
            if(toggle){
                toggle = 0;
            }else{
                toggle = 1;
            }
            ind_led= 0;
            nrf_gpio_pin_write(LED3,toggle);
        }

        ind_led++;


        // Increment compare value to set the beat
        NRF_RTC1->CC[0] = NRF_RTC1->COUNTER + RTC_COUNT_MAX;


        // Read back event register to ensure we have cleared it before exiting IRQ handler.
        dummy = NRF_RTC1->EVENTS_COMPARE[0];
        dummy;
    }
}


void saadc_init(){


    // Enable SAADC
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

    //Enable END event only
    NRF_SAADC->INTEN =  ( SAADC_INTEN_STARTED_Enabled << SAADC_INTEN_STARTED_Pos);

    //Setup DMA buffer
    NRF_SAADC->RESULT.MAXCNT = DMA_COUNT;
    NRF_SAADC->RESULT.PTR = (uint32_t)&result1;
    NRF_SAADC->TASKS_START = 1;
    while(NRF_SAADC->EVENTS_STARTED == 0);
}

void ppi_init(){
    //Short end to start to restart the SAADC
    NRF_PPI->CH[0].EEP = (uint32_t)&NRF_SAADC->EVENTS_END;
    NRF_PPI->CH[0].TEP = (uint32_t)&NRF_SAADC->TASKS_START;

    //Use the RTC events to sample
    NRF_PPI->CH[1].EEP = (uint32_t)&NRF_RTC1->EVENTS_COMPARE[0];
    NRF_PPI->CH[1].TEP = (uint32_t)&NRF_SAADC->TASKS_SAMPLE;
    NRF_PPI->CHEN = ( PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos)|( PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos);

}


void clock_init(){

    //Start LF clock
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while((NRF_CLOCK->EVENTS_LFCLKSTARTED == 0));

    //Setup and start RTC
    NRF_RTC1->PRESCALER = 1;
    NRF_RTC1->CC[0] = RTC_COUNT_MAX;
    NRF_RTC1->EVTEN = (RTC_EVTEN_COMPARE0_Enabled << RTC_EVTEN_COMPARE0_Pos) ;
    NRF_RTC1->INTENSET = (RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos)  ;
    NVIC_EnableIRQ(RTC1_IRQn);
    NRF_RTC1->TASKS_START = 1;
}




/**
 * @}
 */
