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

#define COUNT 1
#define DMA_COUNT 0x1000
#define RTC_COUNT_MAX 512
#define RTC_COUNT_MIN 512

#define BLINKS_PER_KWH 10000

int16_t result1[DMA_COUNT] __attribute__((section (".mydata1")));
int16_t result2[DMA_COUNT] __attribute__((section (".mydata2")));

int16_t results[2];

int saadc_tgl = 0;
int ind1 = 0;
int ind2 = 0;
int swindex = 0;
int16_t max = INT16_MIN;
int16_t min = INT16_MAX;
int16_t val = 0;
int16_t blinkStatus = 0;
int16_t lastBlinkStatus = 0;
int16_t hyst = 10;
int16_t blinkCounter = 0;
float blinks_per_minute = 0;
float kwh = 0;
int16_t ticksToAverage = 100;
int16_t ticks = 0;
int16_t rtcoffset = RTC_COUNT_MAX;
float ticks_per_minute = 0;

void SAADC_IRQHandler(void)
{
    volatile uint32_t dummy;
    if(NRF_SAADC->EVENTS_END == 1){

        NRF_SAADC->EVENTS_END = 0;
        NRF_SAADC->TASKS_STOP = 1;
        while(NRF_SAADC->EVENTS_STOPPED == 0);
        NRF_SAADC->EVENTS_STOPPED = 0;

        //Reset index and get kwh
        if(ind1 >= DMA_COUNT){
            ind1  = 0;
        }

		if(ind2 >= DMA_COUNT){
            ind2  = 0;
        }

		if(ticks >=ticksToAverage){
			ticks_per_minute = (float) 32768*60/(float)(rtcoffset);
			blinks_per_minute =  (float) blinkCounter / (float) ticksToAverage *  ticks_per_minute;
			kwh = blinks_per_minute*60/BLINKS_PER_KWH;


			if(blinkCounter < ticksToAverage * 0.3){
				rtcoffset++;
				if(rtcoffset > RTC_COUNT_MAX){
					rtcoffset = RTC_COUNT_MAX;
				}
			}else if (blinkCounter > ticksToAverage * 0.4){
				rtcoffset = rtcoffset-10;
				if(rtcoffset < RTC_COUNT_MIN)
					rtcoffset = RTC_COUNT_MIN;
			}
				
			ticks = 0;


			result2[ind2] = (int16_t) blinks_per_minute;
			result1[ind1] = (int16_t) (kwh*1000);
			ind1++;

			ind2++;

			blinkCounter = 0;
		}


		
        if(results[0] < min){
            min = results[0];
        }

        if(results[0] > max){
            max = results[0];
        }

        results[0] -= (max + min)/2;

		//Detected a blink
		lastBlinkStatus = blinkStatus;
        if(results[0] <  0 - hyst){
            blinkStatus = 0;
        }else if(results[0] > 0 + hyst){
            blinkStatus = 1;
        }
        if(lastBlinkStatus ==0 && blinkStatus == 1){
			blinkCounter++;
        }


		


		//Reduce/Increase min and max to follow slow changes in lighting
        min++;
        max--;
		ticks++;

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
        NRF_RTC1->CC[0] = NRF_RTC1->COUNTER + rtcoffset;


        //Do a sample
        NRF_SAADC->TASKS_START = 1;
        while (NRF_SAADC->EVENTS_STARTED == 0);
        NRF_SAADC->EVENTS_STARTED = 0;
        NRF_SAADC->TASKS_SAMPLE = 1;

        // Read back event register to ensure we have cleared it before exiting IRQ handler.
        dummy = NRF_RTC1->EVENTS_COMPARE[0];
        dummy;
    }
}


void saadc_init(){

    //Configure channel 0
    NRF_SAADC->CH[0].CONFIG = (
        SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        SAADC_CH_CONFIG_TACQ_5us << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos |
        SAADC_CH_CONFIG_GAIN_Gain4  << SAADC_CH_CONFIG_GAIN_Pos |
        SAADC_CH_CONFIG_RESP_Pullup << SAADC_CH_CONFIG_RESP_Pos |
        SAADC_CH_CONFIG_RESN_Pullup << SAADC_CH_CONFIG_RESN_Pos );

    //Setup AIN4 and AIN5 as inputs
    NRF_SAADC->CH[0].PSELP = 0x5;
    NRF_SAADC->CH[0].PSELN = 0x6;


    //Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;
//    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over2x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;
    NRF_SAADC->OVERSAMPLE = 0;

    //Use external timing
    NRF_SAADC->SAMPLERATE = 0;

    //Setup DMA buffer
    NRF_SAADC->RESULT.MAXCNT = 1;
    NRF_SAADC->RESULT.PTR = (uint32_t)&results;


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
    NRF_RTC1->CC[0] = rtcoffset;
    NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    NVIC_EnableIRQ(RTC1_IRQn);
    NRF_RTC1->TASKS_START = 1;
}




/**
 * @}
 */
