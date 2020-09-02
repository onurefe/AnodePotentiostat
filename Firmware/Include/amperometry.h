/**
  * @author     Onur Efe
  */

#ifndef __AMPEROMETRY_H
#define __AMPEROMETRY_H

#include "board.h"

/* Exported constants --------------------------------------------------------*/
#define AMPEROMETRY_MAX_SAMPLING_FREQUENCY      1000.0f
#define AMPEROMETRY_MIN_SAMPLING_FREQUENCY      0.000001f

/* Exported types ------------------------------------------------------------*/
typedef void (*Amperometry_NewDatapointDelegate_t)(float datapoint);
typedef void (*Amperometry_MeasurementCompletedDelegate_t)(void);

typedef struct
{
  uint16_t                                      datapointCount;
  float                                         samplingFrequency;
  float                                         maxRelSamplingFreqErr;
  float                                         potential;
  float                                         equilibriumPeriod;
  Board_TIAFBPath_t                             feedbackPath;
  double                                        currentGainCorrection;
  double                                        currentOffsetCorrection;
  Amperometry_NewDatapointDelegate_t            newDatapointDelegate;
  Amperometry_MeasurementCompletedDelegate_t    measurementCompletedDelegate;
} Amperometry_SetupParams_t;

typedef enum
{
  AMPEROMETRY_STATE_UNINIT                      = 0x00,
  AMPEROMETRY_STATE_READY                       = 0x01,
  AMPEROMETRY_STATE_OPERATING                   = 0x02
} Amperometry_State_t;


/* Exported functions. -------------------------------------------------------*/
/**
  * @Brief      Configures module. Should be called before any other function.
  * 
  * @Param      pSetupParams: Pointer to data structure which holds setup 
  *             parameters.
  */
extern void Amperometry_Setup(Amperometry_SetupParams_t *pSetupParams);

/**
  * @Brief      Starts amperometry.
  */
extern void Amperometry_Start(void);

/**
  * @Brief      Amperometry task executer and event processor.
  */
extern void Amperometry_Execute(void);

/***
  * @Brief      Stops amperometric measurement.
  */
extern void Amperometry_Stop(void);

/***
  * @Brief      Gets module's state.
  *
  * @Return     State of module.
  */
extern Amperometry_State_t Amperometry_GetState(void);

#endif