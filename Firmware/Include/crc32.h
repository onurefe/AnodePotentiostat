#ifndef __CRC32_H
#define __CRC32_H

#include "generic.h"

/* Exported functions --------------------------------------------------------*/
/***
  * @brief      Calculates CRC32 code of the given byte array. Padding is applied 
  *             for sizes which is not multiple of 4.
  *
  * @params     pBuff-> Pointer to buffer.
  *             size-> Size of the buffer.
  *
  * @retval     CRC code.
  */
extern uint32_t CRC32_Calculate(uint8_t *pBuff, uint32_t size);

#endif