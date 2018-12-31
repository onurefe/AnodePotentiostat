/**
  ******************************************************************************
  * @file    eeprom_emulator.h
  * @author  Onur Efe
  * @date    18.03.2017
  * @brief   EEPROM Emulator class interface.
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
  * #Flash sector 10 and 11 shouldn't be used for other purposes. 
  * #Object ID should be within the range 0-FFFEh.
  * #Module should be initialized first.
  * #Flash voltage range other than FLASH_VOLTAGE_RANGE_3 should be defined by using
  * FLASH_VOLTAGE_RANGE_{1|2|3|4} format.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __EEPROM_EMULATOR_H
#define __EEPROM_EMULATOR_H


/* Includes ------------------------------------------------------------------*/
    
#include "generic.h"

    
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Eeprom emulator module initializer. Should be called before any other 
  *         functions at this module could be used.
  *
  * @param  None.
  *
  * @retval None.
  */
extern void EepromEmulator_Init(void);

/**
  * @brief  Reads data object from the flash.
  *
  * @param  objectId: Data object id.
  * @param  offset: Offset of the read start address.
  * @param  maxLength: Maximum read length.
  * @param  pLength: Pointer to return length of the data object.
  * @param  pData: Pointer to return data.
  *
  * @retval True of False(If the object has been found, true).
  */
extern uint8_t EepromEmulator_ReadObject(uint16_t objectId, uint16_t offset, uint16_t maxLength, 
                                        uint16_t *pLength, uint8_t *pData);

/**
  * @brief  Writes data object to the flash memory. 
  *
  * @param  objectId: Id of the data object which is to be written.
  * @param  length: Length of the data object.
  * @param  pData: Pointer of the data.
  *
  * @retval None.
  */
extern void EepromEmulator_WriteObject(uint16_t objectId, uint16_t length, uint8_t *pData);

#endif