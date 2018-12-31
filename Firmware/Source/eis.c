/**
  * @author     Onur Efe
  */

#include <math.h>
#include "middlewares.h"
#include "eis.h"


/* Private definitions -------------------------------------------------------*/
#define VGND_DAC_CODE                           ((MAX_UINT16 + 1) / 2)
#define VGND_DAC_CODE_LF                        (((double)(MAX_UINT16 + 1)) / 2)

#define SIGNAL_RANGE_SAFETY_FACTOR              1.05
#define CALIB_ELEMENT_REAL                      BOARD_CALIB_ELEMENT_IMP_REAL
#define CALIB_ELEMENT_IMG                       BOARD_CALIB_ELEMENT_IMP_IMG

#define START_EVENT                             0x01
#define STOP_EVENT                              0x02
#define EQUILIBRIUM_PERIOD_ELAPSED_EVENT        0x04        
#define MEASUREMENT_COMPLETED_EVENT             0x08

/* Private function prototypes -----------------------------------------------*/
static void subtractOffset(double calibMeasReal, double calibMeasImg, double rawMeasReal,
                           double rawMeasImg, double *pMeasReal, double *pMeasImg);

static void determineDynamicSignalScaling(float amplitudePP, double *pDAC1LSBPotential,
                                          Board_BinarySignalScaling_t *pBinaryScaling,
                                          Board_DecimalSignalScaling_t *pDecimalScaling);

static void calculateImpedance(double adc1LSBCurrent, double signalAmpPP, double xAvr, 
                               double yAvr, double *pReal, double *pImaginary);

static void equilibriumPeriodElapsedEventHandler(void);
static void measurementCompletedEventHandler(void);

/* Private variables ---------------------------------------------------------*/
/* Module control variables. */
static EIS_State_t                      State = EIS_STATE_UNINIT;
uint8_t                                 Events;

/* Modified variables. */
static uint16_t                         MeasurementIndex;

/* Store variables. They aren't modified during measurement. */
static uint32_t                         EquilibriumPeriodInSysTicks;
static float                            SignalAmplitudePP;
static double                           ADC1LSBCurrent;
static double                           SignalDAC1LSBPotential;
static uint16_t                         BiasDACCode;
static Board_BinarySignalScaling_t      BinaryScaling;
static Board_DecimalSignalScaling_t     DecimalScaling;
static Board_TIAFBPath_t                FBPath;
static double                           *pFrequency;
static uint32_t                         *pCycles;
static double                           *pCalibReal;
static double                           *pCalibImg;
static uint16_t                         NumOfMeasurements;

static EIS_MeasurementCompletedDelegate_t       MeasurementCompletedDelegate;
static EIS_NewDatapointDelegate_t       NewDatapointDelegate;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Module setup. Should be called before any other function.
  *
  * @Param      pMeasParams-> Pointer to the measurement parameters struct.
  */
void EIS_Setup(EIS_MeasurementParams_t *pMeasParams)
{
  /* Check for state compatibility. */
  if ((State == EIS_STATE_OPERATING_EQUILIBRIUM) || (State == EIS_STATE_OPERATING_MEASUREMENT))
  {
    ExceptionHandler_ThrowException(\
      "EIS module setup function called when the module is operating.\n");
  }
  
  // Set store variables. These variables are used during measurements.
  FBPath = pMeasParams->analog.feedbackPath;
  pFrequency = pMeasParams->dds.pFrequency;
  pCycles = pMeasParams->dds.pCycles;
  pCalibReal = pMeasParams->pCalibReal;
  pCalibImg = pMeasParams->pCalibImg;
  NumOfMeasurements = pMeasParams->dds.datapointCount;
  NewDatapointDelegate = pMeasParams->newDatapointDelegate;
  MeasurementCompletedDelegate = pMeasParams->measurementCompletedDelegate;
  
  // Calculate signal amplitude peak to peak.
  SignalAmplitudePP = pMeasParams->analog.amplitudeRms * M_ROOT_OF_2;
  
  // Parse ADC 1 LSB current.
  ADC1LSBCurrent = Board_GetADC1LSBCurrent(FBPath);
  
  // Calculate Bias DAC code.
  BiasDACCode = (uint16_t)(VGND_DAC_CODE_LF + pMeasParams->analog.biasPotential / \
    Board_GetBiasDAC1LSBAppliedPotential());
    
  // Determine signal scaling options.
  determineDynamicSignalScaling(SignalAmplitudePP, &SignalDAC1LSBPotential, 
                                &BinaryScaling, &DecimalScaling);
  
  /* Initial setup of EIS core. */
  {
    EISCore_MeasParams_t eis_core;
        
    eis_core.frequency = pFrequency[0];
    eis_core.measurementCompletedDelegate = measurementCompletedEventHandler;
    eis_core.cycles = pCycles[0];
    eis_core.signal_amp_pp_dac_code = (uint16_t)fabs(SignalAmplitudePP / SignalDAC1LSBPotential);
        
    EISCore_Setup(&eis_core);
  }
  
  // Calculate equilibrium period in system ticks.
  EquilibriumPeriodInSysTicks = (uint32_t)pMeasParams->equilibriumPeriod * SYS_TICK_FREQ;
  
  // Reset events flags.
  Events = 0;
  
  // Set state to ready.
  State = EIS_STATE_READY;
}

/***
  * @Brief      Sets start event.
  */
void EIS_Start(void)
{
  Events |= START_EVENT;
}

/***
  * @Brief      EIS module task executer.
  */
void EIS_Execute(void)
{
  /* No operation when not in operating state. */
  if (State == EIS_STATE_UNINIT)
  {
    ExceptionHandler_ThrowException(\
      "EIS module execute function called when the module is not initialized.\n");
  }
  
  // Execute core module. 
  EISCore_Execute();
  
  /* Start event handler. */
  if (Events & START_EVENT)
  {
    if (State == EIS_STATE_READY)
    {
      AlarmClock_Req_t alarm;
      
      // Reset events flags.
      MeasurementIndex = 0;
      Events = 0;
  
      // Turn on analog circuitry.
      Board_TurnOnAnalog();
        
      // Set scaling.
      Board_SetBinarySignalScaling(BinaryScaling);
      Board_SetDecimalSignalScaling(DecimalScaling);
    
      // Set configuration to impedance spectroscopy.
      Board_ConfigureCore(BOARD_CORE_CONFIGURATION_EIS);
        
      // Ensure that all HUB peripherals are deselected. And load pins are activated.
      Board_TIASetnCS();
      Board_TIAResetnLATCH();
        
      Board_DACBiasSetnCS();
      Board_DACBiasResetnLDAC();
        
      Board_DACSignalSetnCS();
      Board_DACSignalResetnLDAC();
        
      // Select feedback path.
      Board_HUBSPIConfigure(BOARD_HUB_CHANNEL_ID_TIA);
      Board_HUBSPIEnable();
      Board_TIAResetnCS();
      Board_TIASelectFBPath(FBPath);
      Board_TIASetnCS();
      Board_TIASetnLATCH();
        
      // Set Bias DAC code.
      Board_HUBSPIConfigure(BOARD_HUB_CHANNEL_ID_DAC_BIAS);
      Board_HUBSPIEnable();
        
      Board_DACBiasResetnCS();
      Board_HUBSPISend(BiasDACCode);
      while (Board_HUBSPIIsBusy());
      Board_DACBiasSetnCS();
      Board_DACBiasSetnLDAC();
        
      Utils_DelayMs(Board_GetDACBiasStabilizationPeriod());
        
      // Set Signal DAC code..
      Board_HUBSPIConfigure(BOARD_HUB_CHANNEL_ID_DAC_SIGNAL);
      Board_HUBSPIEnable();
      Board_DACSignalResetnCS();
      Board_HUBSPISend(VGND_DAC_CODE);
      while (Board_HUBSPIIsBusy());
      Board_DACSignalSetnCS();
        
      /* Set alarm. */
      alarm.period = EquilibriumPeriodInSysTicks;
      alarm.timerExpCB = equilibriumPeriodElapsedEventHandler;
      AlarmClock_SetAlarm(&alarm);
      
      // Enable ADC SPI.
      Board_ADCSPIEnable();
      
      State = EIS_STATE_OPERATING_EQUILIBRIUM;
    }

    Events ^= START_EVENT;
  }
  
  /* Stop event handler. */
  if (Events & STOP_EVENT)
  {
    /* Process or discard event according to state. */
    if (State == EIS_STATE_OPERATING_EQUILIBRIUM)
    {
      /* Cancel alarm. */
      AlarmClock_CancelReq_t req;
      req.timerExpCB = equilibriumPeriodElapsedEventHandler;
      AlarmClock_CancelAlarm(&req);
      
      /* Disable SPIs. */
      Board_HUBSPIDisable();
      Board_ADCSPIDisable();
      
      // Set configuration to off.
      Board_ConfigureCore(BOARD_CORE_CONFIGURATION_OFF);
      
      // Turn off analog circuitry.
      Board_TurnOffAnalog();
      
      State = EIS_STATE_READY;
    }
    else if (State == EIS_STATE_OPERATING_MEASUREMENT)
    {
      Board_HUBSPIDisable();
      Board_ADCSPIDisable();
      
      // Set configuration to off.
      Board_ConfigureCore(BOARD_CORE_CONFIGURATION_OFF);
      
      // Stop submodules.
      EISCore_Stop();
      
      // Turn off analog circuitry.
      Board_TurnOffAnalog();
      
      State = EIS_STATE_READY;
    }

    Events ^= STOP_EVENT;
  }
  
  /* Equilibrium period completed event. */
  if (Events & EQUILIBRIUM_PERIOD_ELAPSED_EVENT)
  {
    /* Process the event if the state is operating equilibrium. */
    if (State == EIS_STATE_OPERATING_EQUILIBRIUM)
    {
      EISCore_Start();
      
      State = EIS_STATE_OPERATING_MEASUREMENT;
    }
    
    Events ^= EQUILIBRIUM_PERIOD_ELAPSED_EVENT;
  }
  
  /* Measurement completed event. */
  if (Events & MEASUREMENT_COMPLETED_EVENT)
  {
    if (State == EIS_STATE_OPERATING_MEASUREMENT)
    {
      double core_real, core_img;
      double imp_real, imp_img;
      
      // Parse result.
      EISCore_GetResult(&core_real, &core_img);
      MeasurementIndex++;
      
      // Calculate impedance.
      calculateImpedance(ADC1LSBCurrent, SignalAmplitudePP, core_real,
                         core_img, &imp_real, &imp_img);
           
      /* If there is a calibration data, apply it. */
      if ((pCalibReal != NULL) && (pCalibImg != NULL))
      {
        subtractOffset(pCalibReal[MeasurementIndex], pCalibImg[MeasurementIndex], 
                       imp_real, imp_img, &imp_real, &imp_img);
      }
          
      // Call callback function, if it's set.
      if (NewDatapointDelegate != NULL)
      {
        NewDatapointDelegate(imp_real, imp_img);
      }
      
      if (MeasurementIndex >= NumOfMeasurements)
      {
        Board_HUBSPIDisable();
        Board_ADCSPIDisable();
        
        // Set configuration to off.
        Board_ConfigureCore(BOARD_CORE_CONFIGURATION_OFF);
        
        // Stop submodules.
        EISCore_Stop();
        
        // Turn off analog circuitry.
        Board_TurnOffAnalog();
        
        // Call callback function, if it's set.
        if (MeasurementCompletedDelegate != NULL)
        {
          MeasurementCompletedDelegate();
        }
        
        State = EIS_STATE_READY;
      }
      else
      { 
        /* Setup for the next measurement of EIS core. */
        EISCore_MeasParams_t eis_core;
      
        eis_core.signal_amp_pp_dac_code = (uint16_t)fabs(SignalAmplitudePP / SignalDAC1LSBPotential);
        eis_core.frequency = pFrequency[MeasurementIndex];
        eis_core.cycles = pCycles[MeasurementIndex];
        eis_core.measurementCompletedDelegate = measurementCompletedEventHandler;
      
        EISCore_Setup(&eis_core);
        
        // Start new measurement. 
        EISCore_Start();
      }
    }
    Events ^= MEASUREMENT_COMPLETED_EVENT;
  }
}

/***
  * @Brief      Sets stop event.
  */
void EIS_Stop(void)
{
  Events |= STOP_EVENT;
}

                                    
/***
  * @Brief      Returns state of the module.
  *
  * @Return     State.
  */
EIS_State_t EIS_GetState(void)
{
  return State;
}

/***
  * @Brief      Creates calibration profile. Uses eeprom emulator module.
  *
  * @Param      eeObjectIdReal-> EEPROM Emulator object id for real part.
  * @Param      eeObjectIdImg-> EEPROM Emulator object id for imaginary part.
  * @Param      pCalibBuffReal-> Pointer to calibration profile real part.
  * @Param      pCalibBuffImg-> Pointer to calibration profile imaginary part.
  * @Param      datapointCount-> Number of datapoints.
  */
void EIS_CreateCalibrationProfile(uint16_t eeObjectIdReal, uint16_t eeObjectIdImg,
                                  double *pCalibBuffReal, double *pCalibBuffImg, 
                                  uint16_t datapointCount)
{
  /* Check if the datapoint count is valid. */
  if (datapointCount > (MAX_UINT16 / sizeof(double)))
  {
    ExceptionHandler_ThrowException("EIS module error when saving data to calibration \
                                     profile.\n");
  }
  
  /* Write calibration profile to ROM. */
  EepromEmulator_WriteObject(eeObjectIdReal, (datapointCount * sizeof(double)),
                             (uint8_t *)pCalibBuffReal);
  
  EepromEmulator_WriteObject(eeObjectIdImg, (datapointCount * sizeof(double)),
                             (uint8_t *)pCalibBuffImg);                    
}

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
uint8_t EIS_LoadCalibrationProfile(uint16_t eeObjectIdReal, uint16_t eeObjectIdImg,
                                   double *pCalibBuffReal, double *pCalibBuffImg, 
                                   uint16_t *pDatapointCount)
{
  uint8_t profile_real_found;
  uint8_t profile_img_found;
  uint16_t length_real;
  uint16_t length_img;
  uint8_t retval;
  
  /* Load calibration profiles. */
  profile_real_found = EepromEmulator_ReadObject(eeObjectIdReal, 0U, (MAX_UINT16 / sizeof(double)), 
                                                 &length_real, ((uint8_t *)pCalibBuffReal));
  
  profile_img_found = EepromEmulator_ReadObject(eeObjectIdImg, 0U, (MAX_UINT16 / sizeof(double)), 
                                                 &length_img, ((uint8_t *)pCalibBuffImg));
  
  /* If everything is normal, calculate datapoint count and return TRUE. Otherwise
    return false. */
  if ((profile_real_found == TRUE) && (profile_img_found == TRUE) && (length_real == length_img))
  {
    *pDatapointCount = length_real / sizeof(double);
    
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
  }
  
  return retval;
}
                                    
/* Private functions ---------------------------------------------------------*/
/***
  * @Brief      Callback function which is triggered when the equilibrium period 
  *             is elapsed.
  */
static void equilibriumPeriodElapsedEventHandler(void)
{
  // Set event flag.
  Events |= EQUILIBRIUM_PERIOD_ELAPSED_EVENT;
}

/***
  * @Brief      Callback function which is triggered when the measurement is 
  *             completed.
  */
static void measurementCompletedEventHandler(void)
{
  // Set event flag.
  Events |= MEASUREMENT_COMPLETED_EVENT;
}

/***
  * @Brief      Determines optimum SNR dynamic scaling.
  *
  * @Param      amplitudePP-> Signal amplitude peak to peak, in units of volts.
  * @Param      pDAC1LSBPotential-> Pointer to DAC 1LSB potential.
  * @Param      pDynamicScaling-> Pointer to return dynamic scaling.
  * @Param      pBinaryScaling-> Binary scaling option.
  * @Param      pDecimalScaling-> Decimal scaling option.
  */
static void determineDynamicSignalScaling(float amplitudePP, double *pDAC1LSBPotential,
                                          Board_BinarySignalScaling_t *pBinaryScaling,
                                          Board_DecimalSignalScaling_t *pDecimalScaling)
{
  double dac1lsb_potential;
  
  const Board_BinarySignalScaling_t BinaryScalingTable[] = \
  {BOARD_BINARY_SIGNAL_SCALING_1_4, BOARD_BINARY_SIGNAL_SCALING_2_4, 
   BOARD_BINARY_SIGNAL_SCALING_3_4, BOARD_BINARY_SIGNAL_SCALING_4_4};

  const Board_DecimalSignalScaling_t DecimalScalingTable[] = \
  {BOARD_DECIMAL_SIGNAL_SCALING_1_100, BOARD_DECIMAL_SIGNAL_SCALING_1_10,
   BOARD_DECIMAL_SIGNAL_SCALING_1_1};

  /* Decision loop. Algorithm tries the smallest range to the biggest range, if the amplitude
    fits into the range. If it fits, records the scaling options and the value, then returns. */
  for (uint8_t i = 0; i < sizeof(DecimalScalingTable) / sizeof(DecimalScalingTable[0]); i++)
  {
    for (uint8_t j = 0; j < sizeof(BinaryScalingTable) / sizeof(BinaryScalingTable[0]); j++)
    {
      dac1lsb_potential = Board_GetSignalDAC1LSBAppliedPotential(BinaryScalingTable[j], 
                                                                 DecimalScalingTable[i]);
        
      /* Check if DAC range is sufficient for this scaling*/
      if ((((double)MAX_UINT16) / SIGNAL_RANGE_SAFETY_FACTOR) >= fabs(amplitudePP / dac1lsb_potential))
      {
        *pDecimalScaling = DecimalScalingTable[i];
        *pBinaryScaling = BinaryScalingTable[j];
        *pDAC1LSBPotential = dac1lsb_potential;
          
        return;
      } // end of if.
    } // end of for 2.
  } // end of for 1.
} // end of function.

/***
  * @Brief      Calculates impedance in units of kilo-ohms.
  *
  * @Param      adc1LSBCurrent->1 LSB Current of the ADC in units of microamps.
  * @Param      signalAmpPP->Peak to peak signal amplitude in units of volts.
  * @Param      xAvr->Weighted average X parsed from the EIS core.
  * @Param      yAvr->Weigthed average Y parsed from the EIS core.
  * @Param      pReal->Pointer to impedance real component.
  * @Param      pImg->Pointer to impedance imaginary component.
  */
static void calculateImpedance(double adc1LSBCurrent, double signalAmpPP, double xAvr,
                               double yAvr, double *pReal, double *pImaginary)
{
  double current_x;
  double current_y;
  
  /* To prevent loss of precision during calculation, cast SumX and SumY to double. */
  /* Transform SumX, SumY to current_sum_x, current_. */
  current_x = 2.0 * adc1LSBCurrent * yAvr * 0.001;
  current_y = 2.0 * adc1LSBCurrent * xAvr * 0.001;
  
  /* Calculate impedance. */
  *pReal = (float)(signalAmpPP * 0.5f * current_x) / \
    (current_x * current_x + current_y * current_y);
  
  *pImaginary = (float)(-signalAmpPP * 0.5f * current_y) / \
    (current_x * current_x + current_y * current_y);
}
                                    
/***
  * @Brief      Process raw measurement. Calibration measurement is used as reference.
  */
static void subtractOffset(double calibMeasReal, double calibMeasImg, double rawMeasReal, double rawMeasImg,
                           double *pMeasReal, double *pMeasImg)
{
  double calib_mult_real;
  double calib_mult_img;
  
  /* Check if the calibration measurement is valid. */
  if ((calibMeasReal == 0.0) && (calibMeasImg == 0.0))
  {
    ExceptionHandler_ThrowException(\
      "EIS module calibration measurement data is invalid.");
  }
    
  /* Calculate calibration multiplier. Which equals to calibElement / calibMeas in 
    complex number system. */
  calib_mult_real = (CALIB_ELEMENT_REAL * calibMeasReal + CALIB_ELEMENT_IMG * calibMeasImg) / \
    (calibMeasReal * calibMeasReal + calibMeasImg * calibMeasImg);
  
  calib_mult_img = (CALIB_ELEMENT_IMG * calibMeasReal - CALIB_ELEMENT_REAL * calibMeasImg) / \
    (calibMeasReal * calibMeasReal + calibMeasImg * calibMeasImg);
  
  /* Calculate measurement results. */
  *pMeasReal = rawMeasReal * calib_mult_real - rawMeasImg * calib_mult_img;
  *pMeasImg = rawMeasImg * calib_mult_real + rawMeasReal * calib_mult_img;
}
 
                                    