/**
  * @author     Onur Efe
  */ 

#ifndef __BIAS_ADJUSTER_H
#define __BIAS_ADJUSTER_H

#include "eis.h"

typedef enum
{
  BIAS_ADJUSTER_STATE_READY           = 0x00,
  BIAS_ADJUSTER_STATE_OPERATING       = 0x01
} BiasAdjuster_State_t;


extern void BiasAdjuster_Start(float referenceVoltage);
extern void BiasAdjuster_Stop(void);
extern BiasAdjuster_State_t BiasAdjuster_GetState(void);

#endif