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
		dummy = NRF_SAADC->EVENTS_END;
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




/**
 * @}
 */
