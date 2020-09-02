/**
  ******************************************************************************
  * @file    utils.h
  * @author  Onur Efe
  * @date    18.03.2017
  * @brief   Utils class interface.
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

#ifndef __UTILS_H
#define __UTILS_H

/* Includes ------------------------------------------------------------------*/
#include "generic.h"

    
/* Exported functions --------------------------------------------------------*/
extern void Utils_DelayMs(uint32_t miliseconds);
extern void Utils_MemoryCopy(uint8_t *pSource, uint8_t *pDestination, uint32_t length);

#endif