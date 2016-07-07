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


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "saadc.h"

void saadc_deinit(){

	for(int i=0;i<8;i++){
		NRF_SAADC->CH[i].CONFIG = 0x0;
		NRF_SAADC->CH[i].PSELP = 0x0;
		NRF_SAADC->CH[i].PSELN = 0x0;
	}

	NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_10bit << SAADC_RESOLUTION_VAL_Pos;
	NRF_SAADC->OVERSAMPLE = NRF_SAADC->OVERSAMPLE = 0;
	
    // No automatic sampling, will trigger with TASKS_SAMPLE.
    NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;
	
	//Clear results done, it might be 1 after calibration
    NRF_SAADC->EVENTS_RESULTDONE = 0;
    NRF_SAADC->EVENTS_END = 0;
    NRF_SAADC->TASKS_STOP = 1;
    while (NRF_SAADC->EVENTS_STOPPED == 0);
    NRF_SAADC->EVENTS_STOPPED = 0;
    NRF_SAADC->ENABLE = 0;
	
}


// Calibrate SAADC
void saadc_calibrate(){

	// Enable SAADC (would capture analog pins if they were used in CH[0].PSELP)
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

	NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over32x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;
	
    //Set the gain you want to calibrate
	NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
    while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0);
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
    while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy <<SAADC_STATUS_STATUS_Pos));

	saadc_deinit();

}

void saadc_sample(){

    NRF_SAADC->TASKS_START = 1;
    while (NRF_SAADC->EVENTS_STARTED == 0);
    NRF_SAADC->EVENTS_STARTED = 0;

    NRF_SAADC->TASKS_SAMPLE = 1;
    while (NRF_SAADC->EVENTS_END == 0);
    NRF_SAADC->EVENTS_END = 0;
}
