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

#define DMA_COUNT            (0x1000)   //RAM buffer to store data
#define RTC_COUNT_MAX        (64)       //Compare value for RTC, sets current
#define RTC_PRESCALE         (1)        //Slow down RTC clock
#define RTC_COUNT_PER_MINUTE (32758)    //RTC frequency
#define TICKS_TO_AVERAGE     (4096)     //How many ticks to average
#define BLINKS_PER_KWH       (10000)    //Setting on utility meter, change to what yours say


//Controlled buffers for watt hour data, and debug data
int16_t result1[DMA_COUNT] __attribute__((section (".mydata1")));
int16_t result2[DMA_COUNT] __attribute__((section (".mydata2")));
int16_t results[2];

int ind1 = 0;
int ind2 = 0;
int16_t max = INT16_MIN;
int16_t min = INT16_MAX;

static int16_t rtc_offset = RTC_COUNT_MAX;

int16_t blink_status = 0;
int16_t last_blink_status = 0;
int16_t adc_hysteresis = 10;
int16_t blink_counter = 0;
float   scalefactor;
float   wh = 0;
int16_t ticks = 0;
float   ticks_per_minute = 0;

void SAADC_IRQHandler(void)
{

    volatile uint32_t dummy;
    if(NRF_SAADC->EVENTS_END == 1 && ticks >= 0){

        //Stop ADC
        NRF_SAADC->EVENTS_END = 0;
        NRF_SAADC->TASKS_STOP = 1;
        while(NRF_SAADC->EVENTS_STOPPED == 0);
        NRF_SAADC->EVENTS_STOPPED = 0;

        //Reset indexes
        if(ind1 >= DMA_COUNT){
            ind1  = 0;
        }

#ifdef DBG_DATA
        if(ind2 >= DMA_COUNT){
            ind2  = 0;
        }
#endif

        //Calculate watt hours
        if(ticks >=TICKS_TO_AVERAGE){

            //Ignore if there are no blinks
            if(blink_counter > 0){
                wh = blink_counter *scalefactor;
                result1[ind1] = (int16_t) (wh);
                ind1++;
            }

            //Control duty cycle, -1 = 50%, -2 = 33% etc...
            ticks = -1;

            blink_counter = 0;
        }else{
			ticks++;
		}

        //Remove median value
        if(results[0] < min){
            min = results[0];
        }
        if(results[0] > max){
            max = results[0];
        }
        results[0] -= (max + min)/2;


        //Detected a blink
        last_blink_status = blink_status;
        if(results[0] <  0 - adc_hysteresis){
            blink_status = 0;
        }else if(results[0] > 0 + adc_hysteresis){
            blink_status = 1;
        }
        if(last_blink_status ==0 && blink_status == 1){
            blink_counter++;
        }

#ifdef DBG_DATA
        result2[ind2] = blink_status;
        ind2++;
#endif

        //Reduce/Increase min and max to follow slow changes in lighting
        min = min + 0.0005;
        max = max - 0.0005;


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

        if(ticks >= 0){
            // Increment compare value with rtc_offset to set the beat
            NRF_RTC1->CC[0] = NRF_RTC1->COUNTER + rtc_offset;

            //Do a sample
            NRF_SAADC->TASKS_START = 1;
            while (NRF_SAADC->EVENTS_STARTED == 0);
            NRF_SAADC->EVENTS_STARTED = 0;
            NRF_SAADC->TASKS_SAMPLE = 1;

        }else{
            // Increment compare value with rtc_offset to set the beat
            NRF_RTC1->CC[0] = NRF_RTC1->COUNTER + rtc_offset*TICKS_TO_AVERAGE;
            ticks++;
        }

        // Read back event register to ensure we have cleared it before exiting IRQ handler.
        dummy = NRF_RTC1->EVENTS_COMPARE[0];
        dummy;
    }
}


void saadc_init(){

    //Configure channel 0
    NRF_SAADC->CH[0].CONFIG = (
        SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        SAADC_CH_CONFIG_TACQ_3us << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_VDD1_4 << SAADC_CH_CONFIG_REFSEL_Pos |
        SAADC_CH_CONFIG_GAIN_Gain1_5 << SAADC_CH_CONFIG_GAIN_Pos |
        SAADC_CH_CONFIG_RESP_Pullup << SAADC_CH_CONFIG_RESP_Pos |
        SAADC_CH_CONFIG_RESN_Pullup << SAADC_CH_CONFIG_RESN_Pos );

    //Setup AIN4 and AIN5 as inputs
    NRF_SAADC->CH[0].PSELP = 0x6;
    NRF_SAADC->CH[0].PSELN = 0x5;


    //Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;
    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over4x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;

    //Use external timing
    NRF_SAADC->SAMPLERATE = 0;

    //Setup DMA buffer
    NRF_SAADC->RESULT.MAXCNT = 1;
    NRF_SAADC->RESULT.PTR = (uint32_t)&results;

    // Enable SAADC
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

    //Enable END event only
    NRF_SAADC->INTEN = ( SAADC_INTEN_END_Enabled << SAADC_INTEN_END_Pos);

    // The blinks must be multiplied with seconds_per_hour/seconds_averaged/blinks_per_kw to get watt hours
    scalefactor = 1000.0 * (3600.0 / ((float) rtc_offset/ ( (float) RTC_COUNT_PER_MINUTE / (float) RTC_PRESCALE) * (float) TICKS_TO_AVERAGE))/ (float) BLINKS_PER_KWH;
}


void clock_init(){

    //Start LF clock
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while((NRF_CLOCK->EVENTS_LFCLKSTARTED == 0));
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;

    //Setup and start RTC
    NRF_RTC1->PRESCALER = RTC_PRESCALE;
    NRF_RTC1->CC[0] = rtc_offset;
    NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    NVIC_EnableIRQ(RTC1_IRQn);
    NRF_RTC1->TASKS_START = 1;
}
