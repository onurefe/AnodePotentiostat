/**
  * @author     Onur Efe
  */

#ifndef __EIS_CORE_H
#define __EIS_CORE_H

/* Include files -------------------------------------------------------------*/
#include "generic.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  EIS_CORE_STATE_UNINIT                 = 0x00,
  EIS_CORE_STATE_READY                  = 0x01,
  EIS_CORE_STATE_OPERATING              = 0x02
} EISCore_State_t;

typedef void (*EISCore_MeasurementCompletedDelegate_t)(void);

typedef struct
{
  double frequency;                      
  uint16_t signal_amp_pp_dac_code;              // Peak to peak amplitude of the DAC code. 
  uint32_t cycles;                              // Number of cycles.
  EISCore_MeasurementCompletedDelegate_t        measurementCompletedDelegate;
} EISCore_MeasParams_t;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Setup function of the EIS core. Function should called before any
  *             other operation.
  *
  * @Param      pMeasParams->Pointer to measurement params struct.
  */
extern void EISCore_Setup(EISCore_MeasParams_t *pMeasParams);

/***
  * @Brief      Sets start event. 
  */
extern void EISCore_Start(void);

/***
  * @Brief      EIS Core module's task executer. Should be called periodically.
  */
extern void EISCore_Execute(void);

/***
  * @Brief      Sets stop event.
  */
extern void EISCore_Stop(void);

/***
  * @Brief      Gets module's state.
  */
extern EISCore_State_t EISCore_GetState(void);

/***
  * @Brief      Gets result.
  */
extern void EISCore_GetResult(double *pAverageX, double *pAverageY);
#endif