#include "sys_time.h"

volatile uint32_t SysTime = 0U;

uint32_t SysTime_GetTick(void)
{
  return SysTime;
}