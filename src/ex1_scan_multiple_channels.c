/*********************************************************************
 *        Copyright (c) 2016 Carsten Wulff Software, Norway 
 * *******************************************************************
 * Created       : wulff at 2016-7-7
 * *******************************************************************
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 ********************************************************************/


#include "ex1_scan_multiple_channels.h"


void ex1_saadc_init(){

	//Configure VDD as input
	NRF_SAADC->CH[0].CONFIG = (
		SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
		SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos |
		SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_GAIN_Gain1_5 << SAADC_CH_CONFIG_GAIN_Pos |
		SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos |
		SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos );

	NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_VDD;
	NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELN_PSELN_NC;
	
	//Configure VDD/2 as input (VDD - VDD/2)
	NRF_SAADC->CH[1].CONFIG = (
		SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
		SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
		SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_GAIN_Gain1_4 << SAADC_CH_CONFIG_GAIN_Pos |
		SAADC_CH_CONFIG_RESN_VDD1_2 << SAADC_CH_CONFIG_RESN_Pos |
		SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos );

	NRF_SAADC->CH[1].PSELP = SAADC_CH_PSELP_PSELP_VDD;
	NRF_SAADC->CH[1].PSELN = SAADC_CH_PSELN_PSELN_NC;
	
	//Configure -VDD as input
	NRF_SAADC->CH[2].CONFIG = (
		SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
		SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
		SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_GAIN_Gain1_5 << SAADC_CH_CONFIG_GAIN_Pos |
		SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos |
		SAADC_CH_CONFIG_RESP_Pulldown << SAADC_CH_CONFIG_RESP_Pos );

	NRF_SAADC->CH[2].PSELP = 0xFE;  //Safe value to enable channel, but still NC
	NRF_SAADC->CH[2].PSELN = SAADC_CH_PSELN_PSELN_VDD;

	//Configure -VDD/2 as input
	NRF_SAADC->CH[3].CONFIG = (
		SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
		SAADC_CH_CONFIG_MODE_Diff << SAADC_CH_CONFIG_MODE_Pos |
		SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_GAIN_Gain1_5 << SAADC_CH_CONFIG_GAIN_Pos |
		SAADC_CH_CONFIG_RESN_VDD1_2 << SAADC_CH_CONFIG_RESN_Pos |
		SAADC_CH_CONFIG_RESP_Pulldown << SAADC_CH_CONFIG_RESP_Pos );

	NRF_SAADC->CH[3].PSELP = 0xFE; //Safe value to enable channel, but still NC
	NRF_SAADC->CH[3].PSELN = SAADC_CH_PSELN_PSELN_NC;

	
	//Configure AVSS as input
	NRF_SAADC->CH[4].CONFIG = (
		SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos |
		SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos |
		SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_TACQ_Pos |
		SAADC_CH_CONFIG_GAIN_Gain1_5 << SAADC_CH_CONFIG_GAIN_Pos |
		SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos |
		SAADC_CH_CONFIG_RESP_Pulldown << SAADC_CH_CONFIG_RESP_Pos );

	NRF_SAADC->CH[4].PSELP = 0xFE; //Safe value to enable channel, but still NC
	NRF_SAADC->CH[4].PSELN = SAADC_CH_PSELN_PSELN_NC;

	
    // Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;
    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over256x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;

	//Setup memory location
    NRF_SAADC->RESULT.MAXCNT = EX1_COUNT;
    NRF_SAADC->RESULT.PTR = (uint32_t)&ex1_result[0];

    // No automatic sampling, will trigger with TASKS_SAMPLE.
    NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;

    // Enable SAADC (would capture analog pins if they were used in CH[0].PSELP)
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;
	
}

float * ex1_postprocess(uint16_t * count){
	
	*count = EX1_COUNT;
	for(int i=0;i<*count;i++){
		ex1_result_f[i] = ex1_result[i]/ex1_factor[i];
	}

	return &ex1_result_f[0];
}
