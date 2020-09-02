/**
  * @author     Onur Efe
  */

#ifndef __VOLTAMMETRY_CORE_H
#define __VOLTAMMETRY_CORE_H

#include "generic.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*VoltammetryCore_NewDatapointDelegate_t)(float datapoint);
typedef void (*VoltammetryCore_MeasurementCompletedDelegate_t)(void);
typedef uint16_t (*VoltammetryCore_GeneratorFunctionInterface_t)(int32_t tickCounter);

typedef struct
{
  uint16_t                                              datapointCount;
  float                                                 samplingFrequency;
  float                                                 equilibriumPeriod;
  float                                                 maxRelSamplingFreqErr;
  VoltammetryCore_NewDatapointDelegate_t                newDatapointDelegate;
  VoltammetryCore_MeasurementCompletedDelegate_t        measurementCompletedDelegate;
  VoltammetryCore_GeneratorFunctionInterface_t          generatorFunctionInterface;
} VoltammetryCore_SetupParams_t;

typedef enum
{
  VOLTAMMETRY_CORE_STATE_UNINIT                 = 0x00,
  VOLTAMMETRY_CORE_STATE_READY                  = 0x01,
  VOLTAMMETRY_CORE_STATE_OPERATING              = 0x02
} VoltammetryCore_State_t;


/* Exported functions. -------------------------------------------------------*/
/***
  * @Brief      Initializes the module. Should be called before any other function.
  *             It's assumed that interrupt priority is always higher than the 
  *             execute function's priority.
  * @Param      pSetupParams: Setup parameters for the module.
  * @Param      pTickPeriod: Pointer to return calculated tick period.
  */
extern void VoltammetryCore_Setup(VoltammetryCore_SetupParams_t *pSetupParams, 
                                  float *pTickPeriod);

/***
  * @Brief      Sets start event. Event will be processed when event processor 
  *             called.
  */
extern void VoltammetryCore_Start(void);

/***
  * @Brief      Module task executer and event processor.
  */
extern void VoltammetryCore_Execute(void);

/***
  * @Brief      Sets stop event. Event will be processed when event processor 
  *             called.
  */
extern void VoltammetryCore_Stop(void);

/***
  * @Brief      Returns state of the voltammetry core module.
  */
extern VoltammetryCore_State_t VoltammetryCore_GetState(void);

#endif