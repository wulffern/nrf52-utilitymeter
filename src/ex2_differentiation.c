//====================================================================
//        Copyright (c) 2016 Carsten Wulff Software, Norway 
// ===================================================================
// Created       : wulff at 2016-7-7
// ===================================================================
// The MIT License (MIT)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
//====================================================================


/*

About: 
  This second example is an hardware implementation of differentiation.
  It will take two samples in rapid succession, d[n] and d[n+1] and do
  data = (d[n] - d[n+1])/2

  It uses scan, with one channel inverted. And it uses Over2x to average the input channels.
  This is not a pulisized feature of the SAADC, but it's what will happen if you enable
  SCAN and OVERSAMPLE without BURST.

  Since this is performed by the hardware SCAN routine the next sample will start
  immediately after the first, thus it's not that easy to control the sample rate between
  d[n] and d[n+2]. The distance between d[n] and d[n+1] will always be the same, but
  the distance between d[n] and d[n+2] will be decided by the sample rate. In this example
  I've tried to control it with SAMPLERATE register.

  The conversion time for two scan channels is TACQ + TCONV, which would be 24us, according to the PS.
  But the PS specifies that TCONV < 2us. Last time I checked the conversion time was 24 clock cycles
  on the 16MHz clock, but you should really check that for the device that you use, it can be different
  for different silicon versions.

  TACQ = 10us = 160 clock cycles
  TCONV = 24 clock cycles
  TSAMPLE = 2x(TACQ + TCONV) = 368 = 0x170

Running:
  - Send 2 via BLE UART
  - Read out RAM with bin/plotfft 0x20002080 8192
  
  
*/

#include "ex2_differentiation.h"

#define EX2_COUNT 1
#define DMA_COUNT 8192

float ex2_result_f[EX2_COUNT];
int16_t ex2_result[DMA_COUNT] __attribute__((section (".myBufSection")));

// factor = RESOLUTION/(VREF x GAIN)/2^MODE[DIFF,SE]
static const float factor[EX2_COUNT] = { 16384.0/2.4/2};

void ex2_saadc_init(){

	//Configure VDD/2 as input (VDD - VDD/2)
    NRF_SAADC->CH[0].CONFIG = (
        SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_GAIN_Gain1_6 << SAADC_CH_CONFIG_GAIN_Pos |
        SAADC_CH_CONFIG_RESN_Pulldown << SAADC_CH_CONFIG_RESN_Pos |
        SAADC_CH_CONFIG_RESP_VDD1_2 << SAADC_CH_CONFIG_RESP_Pos );

    NRF_SAADC->CH[0].PSELP = 0XFE; //Safe value to enable channel, but still NC
    NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELN_PSELN_NC;

   //Configure -VDD/2 as input
    NRF_SAADC->CH[1].CONFIG = (
        SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos |
        SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
        SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
        SAADC_CH_CONFIG_GAIN_Gain1_6 << SAADC_CH_CONFIG_GAIN_Pos |
        SAADC_CH_CONFIG_RESN_VDD1_2 << SAADC_CH_CONFIG_RESN_Pos |
        SAADC_CH_CONFIG_RESP_Pulldown << SAADC_CH_CONFIG_RESP_Pos );

    NRF_SAADC->CH[1].PSELP = 0xFE; //Safe value to enable channel, but still NC
    NRF_SAADC->CH[1].PSELN = SAADC_CH_PSELN_PSELN_NC;

    //Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;
    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over2x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;

	//Setup memory location
    NRF_SAADC->RESULT.MAXCNT = DMA_COUNT;
    NRF_SAADC->RESULT.PTR = (uint32_t)&ex2_result[0];

    //Use internal timer
    NRF_SAADC->SAMPLERATE = (
        SAADC_SAMPLERATE_MODE_Timers << SAADC_SAMPLERATE_MODE_Pos |
        0x170 << SAADC_SAMPLERATE_CC_Pos);

    // Enable SAADC (would capture analog pins if they were used in CH[0].PSELP)
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

}

float * ex2_postprocess(uint16_t * count){
    *count = EX2_COUNT;
    for(int i=0;i<*count;i++){
        ex2_result_f[i] = ex2_result[i]/factor[i];
    }

    return &ex2_result_f[0];
}
