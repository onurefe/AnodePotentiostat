/**
  * @author     Onur Efe
  */ 

#include "generic.h"
#include "amperometry.h"
#include "voltammetry_core.h"
#include "middlewares.h"

/* Private constants. --------------------------------------------------------*/
// Virtual ground DAC code.
#define VGND_DAC_CODE                                   ((MAX_UINT16 + 1) / 2)

/* Private function prototypes.-----------------------------------------------*/
static uint16_t generatorFunctionImplementation(int32_t tickCounter);
static void     measurementCompletedEventHandler(void);
static void     newDatapointEventHandler(float datapoint);

/* Private variables.---------------------------------------------------------*/
// During initialization.
static Board_TIAFBPath_t                                FBPath;

// During operation.
static uint16_t                                         SignalDACCode;
static uint16_t                                         BiasDACCode;

// For datapoint process;
static double                                           ADC1LSBCurrent;
static double                                           CurrentGainCorrection;
static double                                           CurrentOffsetCorrection;

// New datapoint delegate.
static Amperometry_NewDatapointDelegate_t               NewDatapointDelegate;

// Measurement callback function pointer.
static Amperometry_MeasurementCompletedDelegate_t       MeasurementCompletedDelegate;

// State.
static Amperometry_State_t                              State = AMPEROMETRY_STATE_UNINIT;

/* Public function implementations -------------------------------------------*/
/**
  * @Brief      Configures module. Should be called before any other function.
  * 
  * @Param      pSetupParams-> Pointer to data structure which holds setup 
  *             parameters.
  */
void Amperometry_Setup(Amperometry_SetupParams_t *pSetupParams)
{
  float tick_period;
  double bias_dac_1lsb_voltage;
  VoltammetryCore_SetupParams_t vcore_setup_params;
  
  /* Check state. */
  if (State == AMPEROMETRY_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Amperometry module setup function called when the module is operating.\n");
  }
 
  // Setup of voltammetry core.
  vcore_setup_params.datapointCount = pSetupParams->datapointCount;
  vcore_setup_params.equilibriumPeriod = pSetupParams->equilibriumPeriod;
  vcore_setup_params.generatorFunctionInterface = generatorFunctionImplementation;
  vcore_setup_params.maxRelSamplingFreqErr = pSetupParams->maxRelSamplingFreqErr;
  vcore_setup_params.measurementCompletedDelegate = measurementCompletedEventHandler;
  vcore_setup_params.newDatapointDelegate = newDatapointEventHandler;
  vcore_setup_params.samplingFrequency = pSetupParams->samplingFrequency;
  VoltammetryCore_Setup(&vcore_setup_params, &tick_period);
  
  // Save feedback path.
  FBPath = pSetupParams->feedbackPath;
  
  // Save correction values.
  CurrentGainCorrection = pSetupParams->currentGainCorrection;
  CurrentOffsetCorrection = pSetupParams->currentOffsetCorrection;
  
  // Get 1LSB voltage of the signal DAC.
  bias_dac_1lsb_voltage = Board_GetBiasDAC1LSBAppliedPotential();
  
  // Set Bias DAC code.
  BiasDACCode = VGND_DAC_CODE + \
                 ((int32_t)(pSetupParams->potential / bias_dac_1lsb_voltage));
  
  // Set Signal DAC code.
  SignalDACCode = VGND_DAC_CODE;
  
  // Calculate ADC 1LSB current.
  ADC1LSBCurrent = Board_GetADC1LSBCurrent(FBPath);

  // Set callback function pointer.
  MeasurementCompletedDelegate = pSetupParams->measurementCompletedDelegate;
  NewDatapointDelegate = pSetupParams->newDatapointDelegate;
  
  State = AMPEROMETRY_STATE_READY;
}

/**
  * @Brief      Starts amperometry.
  */
void Amperometry_Start(void)
{
  // Guard for improper calls.
  if (State != AMPEROMETRY_STATE_READY)
  {
    ExceptionHandler_ThrowException(\
      "Amperometry module Start function called when the module isn't ready.\n");
  }
  
  // Turn on analog circuitry.
  Board_TurnOnAnalog();
      
  // Set scaling to 1/400. Because the effect of signal dac is to be minimized. 
  Board_SetBinarySignalScaling(BOARD_BINARY_SIGNAL_SCALING_1_4);
  Board_SetDecimalSignalScaling(BOARD_DECIMAL_SIGNAL_SCALING_1_100);
  
  // Set configuration to voltammetry.
  Board_ConfigureCore(BOARD_CORE_CONFIGURATION_VOLTAMMETRY);
      
  // Ensure that all HUB peripherals are deselected. And load pins are deactivated.
  Board_TIASetnCS();
  Board_TIASetnLATCH();
      
  Board_DACBiasSetnCS();
  Board_DACBiasSetnLDAC();
      
  Board_DACSignalSetnCS();
  Board_DACSignalSetnLDAC();
      
 // Select feedback path.
  Board_HUBSPIConfigure(BOARD_HUB_CHANNEL_ID_TIA);
  Board_HUBSPIEnable();
  
  Board_TIAResetnCS();
  Board_TIASelectFBPath(FBPath);
  Board_TIASetnCS();
  
  Board_TIAResetnLATCH();
  Board_TIASetnLATCH();
  
  // Set Bias DAC code.
  Board_HUBSPIConfigure(BOARD_HUB_CHANNEL_ID_DAC_BIAS);
  Board_HUBSPIEnable();
      
  Board_DACBiasResetnCS();
  Board_HUBSPISend(BiasDACCode);
  while (Board_HUBSPIIsBusy());
  Board_DACBiasSetnCS();
  Board_DACBiasResetnLDAC();
  Board_DACBiasSetnLDAC();
      
  // Set Signal DAC code..
  Board_HUBSPIConfigure(BOARD_HUB_CHANNEL_ID_DAC_SIGNAL);
  Board_HUBSPIEnable();
  Board_DACSignalResetnCS();
  Board_HUBSPISend(VGND_DAC_CODE);
  while (Board_HUBSPIIsBusy());
  Board_DACSignalResetnLDAC();
  Board_DACSignalSetnLDAC();
  
  // Wait bias dac to stabilize.
  Utils_DelayMs(Board_GetDACBiasStabilizationPeriod());
  
  // Enable ADC SPI.
  Board_ADCSPIEnable();
      
  // Start voltammetry core.
  VoltammetryCore_Start();
      
  State = AMPEROMETRY_STATE_OPERATING;
}

/**
  * @Brief      Amperometry task executer and event processor.
  */
void Amperometry_Execute(void)
{
  // If not operating, return.
  if (State != AMPEROMETRY_STATE_OPERATING)
  {
    return;
  }
  
  // Run subthreads.
  VoltammetryCore_Execute();
}

/***
  * @Brief      Stops amperometric measurement.
  */
void Amperometry_Stop(void)
{
  // Guard for improper calls.
  if (State != AMPEROMETRY_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Amperometry module Stop function called when the module isn't operating.\n");
  }
  
  // Set configuration to off.
  Board_ConfigureCore(BOARD_CORE_CONFIGURATION_OFF);
      
  // Stop submodules.
  VoltammetryCore_Stop();
  
  // Turn off analog circuitry.
  Board_TurnOffAnalog();
  
  Board_HUBSPIDisable();
  Board_ADCSPIDisable();
  
  State = AMPEROMETRY_STATE_READY;
}

/***
  * @Brief      Gets module's state.
  *
  * @Return     State of module.
  */
Amperometry_State_t Amperometry_GetState(void)
{
  return State;
}

/* Private function implementations. -----------------------------------------*/
/***
  * @Brief      Callback function which is triggered when new datapoint parsed.
  */
static void newDatapointEventHandler(float datapoint)
{
  // Call delegate if it's set.
  if (NewDatapointDelegate != NULL)
  {
    NewDatapointDelegate((float)(ADC1LSBCurrent * datapoint * CurrentGainCorrection + \
                         CurrentOffsetCorrection));
  }
}
                                    
/***
  * @Brief      Callback function which is triggered when the 
  *             measurement is completed.
  */
static void measurementCompletedEventHandler(void)
{
  Board_HUBSPIDisable();
  Board_ADCSPIDisable();
      
  // Set configuration to off.
  Board_ConfigureCore(BOARD_CORE_CONFIGURATION_OFF);

  // Turn off analog circuitry.
  Board_TurnOffAnalog();
      
  // Send signal to the upper layer via callback function.
  if (MeasurementCompletedDelegate != NULL)
  {
    MeasurementCompletedDelegate();
  }
      
  State = AMPEROMETRY_STATE_READY;
}

/***
  * @Brief      Function which is called when voltammetry core updates
  *             generated signal.
  */
static uint16_t generatorFunctionImplementation(int32_t tickCounter)
{
  return SignalDACCode;
}