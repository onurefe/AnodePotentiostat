/**
  * @author     Onur Efe
  */

#ifndef __STM32F4XX_EXTI_EXT_H
#define __STM32F4XX_EXTI_EXT_H

/**
  * @brief  Checks whether the specified EXTI line flag is set or not. Optimized 
  *         version of EXTI_GetFlagStatus function.
  * @param  EXTI_Line: specifies the EXTI line flag to check.
  *          This parameter can be EXTI_Linex where x can be(0..22)
  * @retval Zero or flag value.
  */
static inline uint16_t EXTI_GetFlagStatusOpt(uint32_t EXTI_Line)
{
  return (EXTI->PR & EXTI_Line);
}

/**
  * @brief  Clears the EXTI's line pending flags. Optimized version of the EXTI_ClearFlag
  *         function.
  * @param  EXTI_Line: specifies the EXTI lines flags to clear.
  *          This parameter can be any combination of EXTI_Linex where x can be (0..22)
  * @retval None.
  */
static inline void EXTI_ClearFlagOpt(uint32_t EXTI_Line)
{
  EXTI->PR = EXTI_Line;
}

#endif