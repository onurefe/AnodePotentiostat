/**
  * @author     Onur Efe
  */

#ifndef __ALARM_CLOCK_H
#define __ALARM_CLOCK_H

/* Includes ------------------------------------------------------------------*/
    
#include "generic.h"

    
/* Exported types ------------------------------------------------------------*/

typedef void (*AlarmClock_TimerExpired_CB_t)(void);

typedef struct
{
  AlarmClock_TimerExpired_CB_t  timerExpCB;
  uint32_t                      period;
} AlarmClock_Req_t;
    
typedef struct
{
  AlarmClock_TimerExpired_CB_t  timerExpCB;
} AlarmClock_CancelReq_t;

/* Exported functions --------------------------------------------------------*/

extern void AlarmClock_SetAlarm(AlarmClock_Req_t *pRequest);
extern void AlarmClock_CancelAlarm(AlarmClock_CancelReq_t *pCancelReq);
extern void AlarmClock_Execute(void);

#endif