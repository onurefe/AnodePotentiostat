/**
  * @author     Onur Efe
  */

#ifndef __STM32F4XX_SPI_EXT_H
#define __STM32F4XX_SPI_EXT_H

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
  *         Optimized inline version of the SPI_I2S_ReceiveData function.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6 
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode. 
  * @retval The value of the received data.
  */
__STATIC_INLINE uint16_t SPI_I2S_ReceiveDataOpt(SPI_TypeDef* SPIx)
{
  /* Return the data in the DR register */
  return SPIx->DR;
}

/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral. Optimized inline
            version of the SPI_I2S_SendData function.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6 
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.     
  * @param  Data: Data to be transmitted.
  * @retval None
  */
__STATIC_INLINE void SPI_I2S_SendDataOpt(SPI_TypeDef* SPIx, uint16_t Data)
{
  /* Write in the DR register the data to be sent */
  SPIx->DR = Data;
}

/**
  * @brief  Checks whether the specified SPIx/I2Sx flag is set or not. Optimized
  *         inline version of the SPI_I2S_GetFlagStatus. At this function return 
  *         type has been changed to uint8_t to do further optimization.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6 
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode. 
  * @param  SPI_I2S_FLAG: specifies the SPI flag to check. 
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_FLAG_TXE: Transmit buffer empty flag.
  *            @arg SPI_I2S_FLAG_RXNE: Receive buffer not empty flag.
  *            @arg SPI_I2S_FLAG_BSY: Busy flag.
  *            @arg SPI_I2S_FLAG_OVR: Overrun flag.
  *            @arg SPI_FLAG_MODF: Mode Fault flag.
  *            @arg SPI_FLAG_CRCERR: CRC Error flag.
  *            @arg SPI_I2S_FLAG_TIFRFE: Format Error.
  *            @arg I2S_FLAG_UDR: Underrun Error flag.
  *            @arg I2S_FLAG_CHSIDE: Channel Side flag.  
  * @retval Zero or the shifted one.
  */
__STATIC_INLINE uint16_t SPI_I2S_GetFlagStatusOpt(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{
  /* Return the masked value. If this isn't equal to zero, this means that the flag
    is set */
  return (SPIx->SR & SPI_I2S_FLAG);
}

/**
  * @brief  Clears the SPIx CRC Error (CRCERR) flag. Optimized inline version of
  *         the SPI_I2S_ClearFlag function.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6 
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode. 
  * @param  SPI_I2S_FLAG: specifies the SPI flag to clear. 
  *          This function clears only CRCERR flag.
  *            @arg SPI_FLAG_CRCERR: CRC Error flag.  
  *  
  * @note   OVR (OverRun error) flag is cleared by software sequence: a read 
  *          operation to SPI_DR register (SPI_I2S_ReceiveData()) followed by a read 
  *          operation to SPI_SR register (SPI_I2S_GetFlagStatus()).
  * @note   UDR (UnderRun error) flag is cleared by a read operation to 
  *          SPI_SR register (SPI_I2S_GetFlagStatus()).   
  * @note   MODF (Mode Fault) flag is cleared by software sequence: a read/write 
  *          operation to SPI_SR register (SPI_I2S_GetFlagStatus()) followed by a 
  *          write operation to SPI_CR1 register (SPI_Cmd() to enable the SPI).
  *  
  * @retval None
  */
__STATIC_INLINE void SPI_I2S_ClearFlagOpt(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{
  /* Clear the selected SPI CRC Error (CRCERR) flag */
  SPIx->SR = (uint16_t)~SPI_I2S_FLAG;
}

#endif