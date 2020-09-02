/**
  * @author     Onur Efe
  */
  
#ifndef __STM32F4XX_USART_EXT_H
#define __STM32F4XX_USART_EXT_H

/**
  * @brief  Checks whether the specified USART flag is set or not.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5, 6, 7 or 8 to select the USART or 
  *         UART peripheral.
  * @param  USART_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg USART_FLAG_CTS:  CTS Change flag (not available for UART4 and UART5)
  *            @arg USART_FLAG_LBD:  LIN Break detection flag
  *            @arg USART_FLAG_TXE:  Transmit data register empty flag
  *            @arg USART_FLAG_TC:   Transmission Complete flag
  *            @arg USART_FLAG_RXNE: Receive data register not empty flag
  *            @arg USART_FLAG_IDLE: Idle Line detection flag
  *            @arg USART_FLAG_ORE:  OverRun Error flag
  *            @arg USART_FLAG_NE:   Noise Error flag
  *            @arg USART_FLAG_FE:   Framing Error flag
  *            @arg USART_FLAG_PE:   Parity Error flag
  * @retval The new state of USART_FLAG (RESET or something different).
  */
__STATIC_INLINE uint16_t USART_GetFlagStatusOpt(USART_TypeDef* USARTx, uint16_t USART_FLAG)
{
  return (USARTx->SR & USART_FLAG);
}

#endif