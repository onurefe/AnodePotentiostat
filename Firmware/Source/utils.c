/**
  ******************************************************************************
  * @file    utils.c
  * @author  Onur Efe
  * @date    18.03.2017
  * @brief   Utils class implementation.
  ******************************************************************************
  *
  * Copyright (c) 2017 VIVOSENS BIOTECHNOLOGY
  *
  * This program is free software; you can redistribute it and/or modify it 
  * under the terms of the GNU General Public License as published by the Free 
  * Software Foundation; either version 2 of the License, or (at your option) 
  * any later version. This program is distributed in the hope that it will be 
  * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
  * Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
  *
  ******************************************************************************
  *
  * Caution!!
  *
  * #There may be fatal problems(waiting for a very long time) if the system tick 
  * frequency is close to the processor frequency(100th of the processor frequency or higher).
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "exception_handler.h"
#include "utils.h"


/* External variables --------------------------------------------------------*/
    
extern volatile uint32_t SysTime;

/* Exported functions --------------------------------------------------------*/
void Utils_DelayMs(uint32_t miliseconds)
{
  uint32_t quit_sys_time;
  
  /* Record sys time */
  quit_sys_time = SysTime;
  quit_sys_time += ((uint32_t)((miliseconds * ((float)SYS_TICK_FREQ / 1000U)) + 0.5f));
  
  /* Wait until the given time has passed. */
  while (quit_sys_time != SysTime);
}

void Utils_MemoryCopy(uint8_t *pSource, uint8_t *pDestination, uint32_t length)
{
  if ((pSource != NULL) && (pDestination != NULL)) 
  {
    for (uint32_t i = 0; i < length; i++)
    {
      pDestination[i] = pSource[i];
    }
  }
}