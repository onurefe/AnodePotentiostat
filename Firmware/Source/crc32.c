#include "crc32.h"
#include "stm32f4xx_conf.h"

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
uint32_t CRC32_Calculate(uint8_t *pBuff, uint32_t size)
{
  uint32_t num_of_chunks = size >> 2;
  uint8_t num_of_leap_bytes = (uint8_t)size - (num_of_chunks << 2);
  
  uint32_t crc_code;
  uint32_t chunk;
  
  // Reset data register.
  CRC_ResetDR();
  
  // Process chunks.
  for (uint32_t i = 0; i < num_of_chunks; i++)
  {
    ((uint8_t *)&chunk)[3] = pBuff[i << 2];
    ((uint8_t *)&chunk)[2] = pBuff[(i << 2) + 1];
    ((uint8_t *)&chunk)[1] = pBuff[(i << 2) + 2];
    ((uint8_t *)&chunk)[0] = pBuff[(i << 2) + 3];
    
    crc_code = CRC_CalcCRC(chunk);
  }
  
  // Process the leap bytes.
  if (num_of_leap_bytes != 0)
  {
    chunk = 0;
    
    for (uint8_t i = 0; i < num_of_leap_bytes; i++)
    {
      ((uint8_t *)&chunk)[3 - i] = pBuff[(num_of_chunks << 2) + i];
    }
    
    crc_code = CRC_CalcCRC(chunk);
  }
  
  return crc_code;
}