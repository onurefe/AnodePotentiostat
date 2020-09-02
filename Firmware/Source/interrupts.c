/**
  * @author     Onur Efe
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "generic.h"

/* External variables --------------------------------------------------------*/
extern volatile uint32_t SysTime;

/* External functions --------------------------------------------------------*/
extern void Board_PowerButtonPressedISR(void);
extern void Board_ADCBusyPinReleasedISR(void);
extern void PacketManager_UARTIsr(void);    
extern void VoltammetryCore_TimerTickISR(void);
extern void EISCore_TimerTickISR(void);
extern void HCI_Isr(void);

/* Interrupt handlers --------------------------------------------------------*/
void SysTick_Handler(void)
{
  SysTime++;
}

void EXTI15_10_IRQHandler(void)
{
  /* If EXTI_Line13 interrupt occured; */
  if (EXTI_GetFlagStatusOpt(EXTI_Line13))
  {
    EXTI_ClearFlagOpt(EXTI_Line13);
    Board_PowerButtonPressedISR();
  }
}

void EXTI9_5_IRQHandler(void)
{
  /* If EXTI Line 6 interrupt occured; */
  if (EXTI_GetFlagStatusOpt(EXTI_Line6))
  {
    EXTI_ClearFlagOpt(EXTI_Line6);
    Board_ADCBusyPinReleasedISR();
  }
  
  /* If EXTI Line 8 interrupt occured; */
  if (EXTI_GetFlagStatusOpt(EXTI_Line8))
  {
    EXTI_ClearFlagOpt(EXTI_Line8);
    //HCI_Isr();
  }
}

void TIM5_IRQHandler(void)
{
  /* If tim5 update interrupt occured; */
  if (TIM_GetFlagStatusOpt(TIM5, TIM_IT_Update))
  {
    TIM_ClearFlagOpt(TIM5, TIM_IT_Update);
    VoltammetryCore_TimerTickISR();
  }
}

void TIM2_IRQHandler(void)
{
  /* If tim2 update interrupt occured; */
  if (TIM_GetFlagStatusOpt(TIM2, TIM_IT_Update))
  {
    TIM_ClearFlagOpt(TIM2, TIM_IT_Update);
    EISCore_TimerTickISR();
  }
}

void UART4_IRQHandler(void)
{
  PacketManager_UARTIsr();
}