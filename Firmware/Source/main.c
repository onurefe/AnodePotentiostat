#include "generic.h"
#include "middlewares.h"
#include "peripheral_init.h"
#include "device_manager.h"

/* Private variables ---------------------------------------------------------*/
int main(void)
{
  // Set priority grouping to all bits for preemption priority.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  // Setup system clock sources.
  SystemInit();
  
  // Set and init system tick.
  SysTick_Config(SystemCoreClock / SYS_TICK_FREQ);
  
  /* Init peripherals */
  Init_CRC();
  Init_GPIO();
  Init_EXTI();
  Init_SYSCFG();
  Init_TIM();
  Init_SPI();
  Init_USART();
  Init_NVIC();
  
  // Setup of the device manager.
  DeviceManager_Setup();
  
  // Start of the device manager.
  DeviceManager_Start();
  
  /* Infinite loop */
  while (1)
  {
    DeviceManager_Execute();
    AlarmClock_Execute();
  }
}