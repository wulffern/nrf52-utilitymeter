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
#include "hal_utility.h"

//nRF52 DK
#define LED1 17
#define LED2 18
#define LED3 19
#define LED4 20

#define RESTORE_SOFTDEVICE 1
#undef SOFTDEVICE_PRESENT




int main(void)
{
   //Enable DC/DC
	NRF_POWER->DCDCEN = 1;

	hal_utility_init();

    // Enter main loop.
    for (;;)
    {
		hal_utility_state_machine();
    }
}


/**
 * @}
 */
