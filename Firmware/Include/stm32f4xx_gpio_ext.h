/**
  * @author     Onur Efe
  */

#ifndef __STM32F4XX_GPIO_EXT_H
#define __STM32F4XX_GPIO_EXT_H

/**
  * @brief  Sets the selected data port bits. Optimized static inline version of
  *         GPIO_SetBits function.
  * @note   This functions uses GPIOx_BSRR register to allow atomic read/modify 
  *         accesses. In this way, there is no risk of an IRQ occurring between
  *         the read and the modify access.
  * @param  GPIOx: where x can be (A..K) to select the GPIO peripheral for STM32F405xx/407xx and STM32F415xx/417xx devices
  *                      x can be (A..I) to select the GPIO peripheral for STM32F42xxx/43xxx devices.
  *                      x can be (A, B, C, D and H) to select the GPIO peripheral for STM32F401xx devices. 
  * @param  GPIO_Pin: specifies the port bits to be written.
  *          This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None.
  */
__STATIC_INLINE void GPIO_SetBitsOpt(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  GPIOx->BSRRL = GPIO_Pin;
}

/**
  * @brief  Clears the selected data port bits. Optimized static inline version of
  *         GPIO_ResetBits function.
  * @note   This functions uses GPIOx_BSRR register to allow atomic read/modify 
  *         accesses. In this way, there is no risk of an IRQ occurring between
  *         the read and the modify access.
  * @param  GPIOx: where x can be (A..K) to select the GPIO peripheral for STM32F405xx/407xx and STM32F415xx/417xx devices
  *                      x can be (A..I) to select the GPIO peripheral for STM32F42xxx/43xxx devices.
  *                      x can be (A, B, C, D and H) to select the GPIO peripheral for STM32F401xx devices. 
  * @param  GPIO_Pin: specifies the port bits to be written.
  *          This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
__STATIC_INLINE void GPIO_ResetBitsOpt(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  GPIOx->BSRRH = GPIO_Pin;
}

#endif