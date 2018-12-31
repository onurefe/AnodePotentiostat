/**
  * @author     Onur Efe
  */

#ifndef __EIS_H
#define __EIS_H

/* Included files ------------------------------------------------------------*/
#include "generic.h"
#include "board.h"
#include "eis_core.h"

/* Exported constants --------------------------------------------------------*/
#define EIS_MAX_FREQUENCY       100000.0f
#define EIS_MIN_FREQUENCY       0.001f

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  EIS_STATE_UNINIT                      = 0x00,
  EIS_STATE_READY                       = 0x01,
  EIS_STATE_OPERATING_EQUILIBRIUM       = 0x02,
  EIS_STATE_OPERATING_MEASUREMENT       = 0x03
} EIS_State_t;

typedef void (*EIS_MeasurementCompletedDelegate_t)(void);
typedef void (*EIS_NewDatapointDelegate_t)(double impReal, double impImg);

typedef struct
{
  float                 amplitudeRms;
  float                 biasPotential;
  Board_TIAFBPath_t     feedbackPath;
} EIS_Analog_t;

typedef struct
{
  double                *pFrequency;
  uint32_t              *pCycles;
  uint16_t              datapointCount;
} EIS_DDS_t;

typedef struct
{
  EIS_Analog_t          analog;
  EIS_DDS_t             dds;
  float                 equilibriumPeriod;
  EIS_NewDatapointDelegate_t            newDatapointDelegate;
  EIS_MeasurementCompletedDelegate_t    measurementCompletedDelegate;
  double                *pCalibReal;
  double                *pCalibImg;
} EIS_MeasurementParams_t;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Module setup. Should be called before any other function.
  *
  * @Param      pMeasParams-> Pointer to the measurement parameters struct.
  */
extern void EIS_Setup(EIS_MeasurementParams_t *pMeasParams);

/***
  * @Brief      Sets start event.
  */
extern void EIS_Start(void);

/***
  * @Brief      EIS module task executer.
  */
extern void EIS_Execute(void);

/***
  * @Brief      Sets stop event.
  */
extern void EIS_Stop(void);

/***
  * @Brief      Returns state of the module.
  *
  * @Return     State.
  */
extern EIS_State_t EIS_GetState(void);

/***
  * @Brief      Loads calibration profile.
  *
  * @Param      eeObjectIdReal-> EEPROM Emulator object id for real part.
  * @Param      eeObjectIdImg-> EEPROM Emulator object id for imaginary part.
  * @Param      pCalibBuffReal-> Pointer to parsed calibration profile real part.
  * @Param      pCalibBuffImg-> Pointer to parsed calibration profile imaginary part.
  * @Param      pDatapointCount-> Pointer to return number of datapoints.
  * 
  * @Return     Profile found of not(TRUE of FALSE).
  */
extern uint8_t EIS_LoadCalibrationProfile(uint16_t eeObjectIdReal, uint16_t eeObjectIdImg,
                                          double *pCalibBuffReal, double *pCalibBuffImg,
                                          uint16_t *pDatapointCount);

/***
  * @Brief      Creates calibration profile. Uses eeprom emulator module.
  *
  * @Param      eeObjectIdReal-> EEPROM Emulator object id for real part.
  * @Param      eeObjectIdImg-> EEPROM Emulator object id for imaginary part.
  * @Param      pCalibBuffReal-> Pointer to calibration profile real part.
  * @Param      pCalibBuffImg-> Pointer to calibration profile imaginary part.
  * @Param      datapointCount-> Number of datapoints.
  */
extern void EIS_CreateCalibrationProfile(uint16_t eeObjectIdReal, uint16_t eeObjectIdImg,
                                         double *pCalibBuffReal, double *pCalibBuffImg, 
                                         uint16_t datapointCount);

#endif