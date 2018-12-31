/**
  * @author     Onur Efe
  */

/* Includes ------------------------------------------------------------------*/

#include "alarm_clock.h"
#include "sys_time.h"
#include "exception_handler.h"

/* Definitions ---------------------------------------------------------------*/
#define MAX_SIMULTANEOUS_INSTANCES      10U

/* Private typedefs ----------------------------------------------------------*/
typedef struct
{
  AlarmClock_TimerExpired_CB_t  timerExpCB;
  uint32_t                      timeoutValue;
} AlarmInstance_t;

/* Private variables ---------------------------------------------------------*/
static AlarmInstance_t  InstanceList[MAX_SIMULTANEOUS_INSTANCES] = {{NULL, 0}};

/* Exported functions --------------------------------------------------------*/
void AlarmClock_SetAlarm(AlarmClock_Req_t *pRequest)
{
  uint8_t index = 0;
  uint32_t hold_sys_time;
  
  hold_sys_time = SysTime_GetTick();
  
  /* Search for an empty space in the instance list. */
  while (InstanceList[index].timerExpCB != NULL)
  {
    /* If the list is full, call exception handler. */
    if (++index == MAX_SIMULTANEOUS_INSTANCES)
    {
      ExceptionHandler_ThrowException("Alarm clock buffer overflowed.\n");
    }
  }
  
  /* Set instance. */
  InstanceList[index].timeoutValue = pRequest->period + hold_sys_time;
  InstanceList[index].timerExpCB = pRequest->timerExpCB;
}

void AlarmClock_CancelAlarm(AlarmClock_CancelReq_t *pCancelReq)
{
  /* Find and cancel the request. */
  for (uint8_t index = 0; index < MAX_SIMULTANEOUS_INSTANCES; index++)
  {
    if (InstanceList[index].timerExpCB == pCancelReq->timerExpCB)
    {
      InstanceList[index].timerExpCB = NULL;
    }
  }
}

void AlarmClock_Execute(void)
{
  uint32_t hold_sys_time;
  
  hold_sys_time = SysTime_GetTick();
  
  /* Look for all the slots, if there is an expired timer call it's callback function. */
  for (uint8_t index = 0; index < MAX_SIMULTANEOUS_INSTANCES; index++)
  {
     if ((InstanceList[index].timeoutValue <= hold_sys_time) && \
        (InstanceList[index].timerExpCB != NULL))
    {
      InstanceList[index].timerExpCB();
      InstanceList[index].timerExpCB = NULL;
    }
  }
}