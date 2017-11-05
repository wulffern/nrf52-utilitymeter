/*********************************************************************
 *        Copyright (c) 2017 Carsten Wulff Software, Norway 
 * *******************************************************************
 * Created       : wulff at 2017-08-26
 * *******************************************************************
 *  The MIT License (MIT)
 * 
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 ********************************************************************/


#include "hal_utility.h"

//Controlled buffers for watt hour data, and debug data
int16_t results[2];

sensor_state_t state;

int ind1 = 0;
int ind2 = 0;
int16_t max = INT16_MIN;
int16_t min = INT16_MAX;
uint16_t rtc_offset = RTC_COUNT_MAX;

uint16_t blink_status = 0;
uint16_t last_blink_status = 0;
uint16_t adc_hysteresis = 50;
uint16_t blink_counter = 0;

uint16_t  scalefactor ;
uint16_t   w = 0;
uint16_t ticks = 0;

int16_t result1[DMA_COUNT] __attribute__((section (".mydata1")));
uint16_t result2[DMA_COUNT] __attribute__((section (".mydata2")));

uint8_t adv_pdu[36 + 3] =
{
    0x42, 0x24, 0x00,
    0xE2, 0xA3, 0x01, 0xE7, 0x61, 0xF7,
    15,    9,  'c', 'a' ,  'w',  'u',  ':',  ' ', 'w',   ':', ' ',     0,    0,  0,  0,  0,
    12, 0xff, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0, 0x01, 0x02, 0x03, 0x04, 0xC3
};

static bool volatile m_radio_isr_called;    /* Indicates that the radio ISR has executed. */


//--------------------------------------------------------------
// Waits for the next NVIC event.
//--------------------------------------------------------------
#ifdef __GNUC__
static void __INLINE cpu_wfe(void)
#else
    static void __forceinline cpu_wfe(void)
#endif
{
    __WFE();
    __SEV();
    __WFE();
}


//--------------------------------------------------------------
// Capture the adc samples, filter, and calculate the watt hours
// at regular intervals
//--------------------------------------------------------------
void SAADC_IRQHandler(void)
{

    volatile uint32_t dummy;
    if(NRF_SAADC->EVENTS_END == 1 && ticks >= 0){

        //Stop ADC
        NRF_SAADC->EVENTS_END = 0;
        NRF_SAADC->TASKS_STOP = 1;
        while(NRF_SAADC->EVENTS_STOPPED == 0);
        NRF_SAADC->EVENTS_STOPPED = 0;

        //Reset adc/watt_hour results buffers
        if(ind1 >= DMA_COUNT) ind1 = 0;
        if(ind2 >= DMA_COUNT) ind2 = 0;


        //Calculate watt hours
        if(ticks >=TICKS_TO_AVERAGE){

            //Ignore results if there are no blinks
            if(blink_counter > 0){
                w = blink_counter * scalefactor;
                result2[ind2] = w;
                ind2++;
            }


            uint16_t watt = (uint16_t) w;

            //Make the watt hours easy to read, use one decimal number per nibble
            char str[5];
            sprintf(str, "%05d", watt);
            adv_pdu[20] = str[0];
            adv_pdu[20+1] = str[0+1];
            adv_pdu[20+2] = str[0+2];
            adv_pdu[20+3] = str[0+3];
            adv_pdu[20+4] = str[0+4];

            uint16_t tmp = watt;
            uint8_t w_e5 = tmp/1e5;
            tmp -= w_e5*1e5;
            uint8_t w_e4 = tmp/1e4;
            tmp -= w_e4*1e4;
            uint8_t w_e3 = tmp/1e3;
            tmp -= w_e3*1e3;
            uint8_t w_e2 = tmp/1e2;
            tmp -= w_e2*1e2;
            uint8_t w_e1 = tmp/1e1;
            tmp -= w_e1*1e1;
            uint8_t w_e0 = tmp;
            adv_pdu[32] = 0xFF;
            adv_pdu[33] = (w_e5 << 4) | w_e4 ;
            adv_pdu[34] = (w_e3 << 4) | w_e2 ;
            adv_pdu[35] = (w_e1 << 4) | w_e0;

            //Send the hex value also
            adv_pdu[36] = 0xFF;
            adv_pdu[37] = (uint8_t) (watt >> 8);
            adv_pdu[38] = (uint8_t) watt;


            //Let the state machine know that we're ready to transmitt
            state = POWER_DATA_READY;

            //Control duty cycle, -1 = 50%, -2 = 33% etc...
            ticks = 0;

            //Reduce/Increase min and max to follow slow changes in lighting
            min = min + 1;
            max = max - 1;

            blink_counter = 0;
        }else{
            ticks++;
        }

        //Remove median value, helps with offset
        if(results[0] < min){
            min = results[0];
        }
        if(results[0] > max){
            max = results[0];
        }
        results[0] -= (max + min)/2;

        //Store SAADC values
        result1[ind1] = results[0];
        ind1++;


        //Detected a blink, basically a zero cross detection
        last_blink_status = blink_status;
        if(results[0] <  0 - adc_hysteresis){
            blink_status = 0;
        }else if(results[0] > 0 + adc_hysteresis){
            blink_status = 1;
        }
        if(last_blink_status ==1 && blink_status == 0){
            blink_counter++;
        }




        // Read back event register to ensure we have cleared it before exiting IRQ handler.
        dummy = NRF_SAADC->EVENTS_END;
        dummy;

    }

}

//--------------------------------------------------------------
// Keep the beat.
//--------------------------------------------------------------
void RTC2_IRQHandler(void)
{
    volatile uint32_t dummy;
    if (NRF_RTC2->EVENTS_COMPARE[0] == 1)
    {
        NRF_RTC2->EVENTS_COMPARE[0] = 0;

        if(ticks >= 0){
            // Increment compare value with rtc_offset to set the beat
            NRF_RTC2->CC[0] = NRF_RTC2->COUNTER + rtc_offset;

            //Skip SAADC sampling if we're doing anything else
            if(state == SAADC_CAPTURE){
                //Do a sample
                NRF_SAADC->TASKS_START = 1;
                while (NRF_SAADC->EVENTS_STARTED == 0);
                NRF_SAADC->EVENTS_STARTED = 0;
                NRF_SAADC->TASKS_SAMPLE = 1;
            }

        }else{
            //Sleep for a while to reduce current consumption
            NRF_RTC2->CC[0] = NRF_RTC2->COUNTER + rtc_offset*TICKS_TO_AVERAGE;
            ticks++;
        }

        // Read back event register to ensure we have cleared it before exiting IRQ handler.
        dummy = NRF_RTC2->EVENTS_COMPARE[0];
        dummy;
    }
}


//--------------------------------------------------------------
// Configure differential measurement of AIN4 and AIN5
//--------------------------------------------------------------
void hal_utility_saadc_init(){

    //Configure channel 0
    NRF_SAADC->CH[0].CONFIG = (
        SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        SAADC_CH_CONFIG_TACQ_5us << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos |
        SAADC_CH_CONFIG_GAIN_Gain1_2 << SAADC_CH_CONFIG_GAIN_Pos |
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
    scalefactor = 1000.0 * (3600.0 / ( (TICKS_TO_AVERAGE * rtc_offset) / (  RTC_COUNT_PER_SECOND /  (RTC_PRESCALE + 1) ) ) )/ BLINKS_PER_KWH;

    NVIC_ClearPendingIRQ(SAADC_IRQn);
    NVIC_EnableIRQ(SAADC_IRQn);
}

//--------------------------------------------------------------
// Setup XOSC32K
//--------------------------------------------------------------
void hal_utility_clock_init(){
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while((NRF_CLOCK->EVENTS_LFCLKSTARTED == 0));
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

//--------------------------------------------------------------
// Setup RTC, and interrupts
//--------------------------------------------------------------
void hal_utility_rtc_init(){
    NRF_RTC2->PRESCALER = RTC_PRESCALE;
    NRF_RTC2->CC[0] = rtc_offset;
    NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    NVIC_EnableIRQ(RTC2_IRQn);
    NRF_RTC2->TASKS_START = 1;
}


//--------------------------------------------------------------
// Setup everything, and reset radio
//--------------------------------------------------------------
void hal_utility_init(){

    state = SAADC_CAPTURE_INIT;

    hal_utility_saadc_init();
    hal_utility_clock_init();
    hal_utility_rtc_init();

#ifdef DBG_STATES
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);
    nrf_gpio_cfg_output(LED3);
    nrf_gpio_cfg_output(LED4);
    nrf_gpio_pin_write(LED1,1);
    nrf_gpio_pin_write(LED2,1);
    nrf_gpio_pin_write(LED3,1);
    nrf_gpio_pin_write(LED4,1);
#endif

    hal_radio_reset();
}


//--------------------------------------------------------------
// Control the current state
//--------------------------------------------------------------
void hal_utility_state_machine(){

    switch(state){
    case SAADC_CAPTURE_INIT:
        state = SAADC_CAPTURE;

#ifdef DBG_STATES
        nrf_gpio_pin_write(LED1,1);
#endif
        break;
    case SAADC_CAPTURE:
        cpu_wfe();
        break;
    case POWER_DATA_READY:
        state = ADVERTIZE;
        NRF_CLOCK->TASKS_HFCLKSTART = 1;
        while(NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
        NRF_CLOCK->EVENTS_HFCLKSTARTED =0;
        break;
    case ADVERTIZE:
#ifdef DBG_STATES
        nrf_gpio_pin_write(LED1,0);
#endif

        send_one_packet(37);
        send_one_packet(38);
        send_one_packet(39);

#ifdef DBG_STATES
        nrf_gpio_pin_write(LED1,1);
#endif

        NRF_CLOCK->TASKS_HFCLKSTOP = 1;
        state = SAADC_CAPTURE_INIT;
        break;
    }

}


//--------------------------------------------------------------
// Sends an advertising PDU on the given channel index.
//--------------------------------------------------------------
void send_one_packet(uint8_t channel_index)
{
    uint8_t i;

    m_radio_isr_called = false;
    hal_radio_channel_index_set(channel_index);
    hal_radio_send(adv_pdu);
    while ( !m_radio_isr_called )
    {
        cpu_wfe();
    }

    for ( i = 0; i < 9; i++ )
    {
        __NOP();
    }
}


//--------------------------------------------------------------
// Shutdown radio when it's done
//--------------------------------------------------------------
void RADIO_IRQHandler(void)
{
    NRF_RADIO->EVENTS_DISABLED = 0;
    m_radio_isr_called = true;
}
