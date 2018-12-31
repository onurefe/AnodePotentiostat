/**
  * @author     Onur Efe
  */ 

#include "voltammetry_core.h"
#include "math.h"
#include "board.h"
#include "middlewares.h"
    
/* Private constants ---------------------------------------------------------*/
#define MAX_TICK_FREQUENCY                      50000U

#define DATA_SAMPLED_EVENT                      0x01

#define VGND_DAC_CODE                           ((MAX_UINT16 + 1) / 2)

#define DOWNSAMPLING_FILTER_MAGIC_NUMBER        1.333

/* Public variables ----------------------------------------------------------*/
uint16_t ADCConversionResult[2];

/* Private function prototypes -----------------------------------------------*/
static uint16_t defaultGeneratorFunctionImplementation(int32_t tickValue);
static void adjustParameters(uint32_t timClockFrequency, uint32_t timMaxReload,
                             uint16_t timMaxPrescaler, float requiredSamplingFreq,
                             float maxRelSamplingFreqErr, uint32_t *pTimReload,
                             uint16_t *pTimPrescaler, uint32_t *pDownsamplingNumber,
                             float *pTickPeriod);

/* Private variables ---------------------------------------------------------*/
// During operation.
static uint16_t                 NumberOfDatapoints;
static uint16_t                 DatapointCounter;

static int32_t                  TickCounterReset;
static int32_t                  TickCounter;
static int32_t                  NextSamplingTick;
static uint32_t                 DownsamplingNumber;
                 
static float                    DownsamplingFilterCoefficient;
static float                    DownsamplingFilterOutput1;
static float                    DownsamplingFilterOutput2;
static float                    DownsamplingFilterOutput3;
static float                    DownsamplingFilterOutput4;

static float                    DownsamplingResult;

static int16_t                  ConversionValue;

// Function pointers.
static VoltammetryCore_NewDatapointDelegate_t           NewDatapointDelegate;
static VoltammetryCore_MeasurementCompletedDelegate_t   MeasurementCompletedDelegate;
static VoltammetryCore_GeneratorFunctionInterface_t     GeneratorFunctionInterface;

// State.
static VoltammetryCore_State_t  State = VOLTAMMETRY_CORE_STATE_UNINIT;

// Events.
static volatile uint8_t         Events;

// Lock status.
static uint8_t                  IsLocked = FALSE;


/* Public function implementations -------------------------------------------*/
/***
  * @Brief      Interrupt service routine for timer period elapsed event. Should 
  *             be exported.
  */
void VoltammetryCore_TimerTickISR(void)
{
  static uint16_t dac_code;
  
  // Discard incompatible operations or race conditions.
  if ((State != VOLTAMMETRY_CORE_STATE_OPERATING) || (IsLocked == TRUE))
  {
    return;
  }
  
  Board_DACSignalSetnCS();
  
  // Get last conversion result.
  ConversionValue = Board_ADCGetValue();
  
  // Trigger next conversion.
  Board_ADCTriggerConvert();
  
  Board_DACSignalResetnCS();
  
  // Filter conversion value with fourth order filter. Filter is cascaded ema filter.
  DownsamplingFilterOutput1 *= (1.0f - DownsamplingFilterCoefficient);
  DownsamplingFilterOutput1 += (ConversionValue * DownsamplingFilterCoefficient);
  
  DownsamplingFilterOutput2 *= (1.0f - DownsamplingFilterCoefficient);
  DownsamplingFilterOutput2 += (DownsamplingFilterOutput1 * DownsamplingFilterCoefficient);
  
  DownsamplingFilterOutput3 *= (1.0f - DownsamplingFilterCoefficient);
  DownsamplingFilterOutput3 += (DownsamplingFilterOutput2 * DownsamplingFilterCoefficient);
  
  DownsamplingFilterOutput4 *= (1.0f - DownsamplingFilterCoefficient);
  DownsamplingFilterOutput4 += (DownsamplingFilterOutput3 * DownsamplingFilterCoefficient);

  // Generate DAC code.
  dac_code = GeneratorFunctionInterface(TickCounter);
  
  // Set Signal DAC value. Sequence may seem weird. But this is used for framing data.
  Board_HUBSPISend(dac_code);
  
  if (TickCounter >= NextSamplingTick)
  {
    NextSamplingTick += DownsamplingNumber;
    DownsamplingResult = DownsamplingFilterOutput4;
    Events |= DATA_SAMPLED_EVENT;                      // Set data sampled event.
  }
  
  TickCounter++;
}

/***
  * @Brief      Initializes the module. Should be called before any other function.
  *             It's assumed that interrupt priority is always higher than the 
  *             execute function's priority.
  * @Param      pSetupParams-> Setup parameters for the module.
  * @Param      pTickPeriod-> Pointer to return calculated tick period.
  */
void VoltammetryCore_Setup(VoltammetryCore_SetupParams_t *pSetupParams, 
                           float *pTickPeriod)
{
  uint32_t tim_reload;
  uint16_t tim_prescaler;
  float tick_period;
  
  /* Check state. */
  if (State == VOLTAMMETRY_CORE_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException("Voltammetry core module setup function called when the module is \
                                     operating.\n");
  }
  
  /* Adjust parameters. */
  adjustParameters(VOLTAMMETRY_CORE_TIMER_FREQUENCY, VOLTAMMETRY_CORE_TIMER_MAX_RELOAD, 
                   VOLTAMMETRY_CORE_TIMER_MAX_PRESCALER, pSetupParams->samplingFrequency, 
                   pSetupParams->maxRelSamplingFreqErr, &tim_reload, &tim_prescaler, 
                   &DownsamplingNumber, &tick_period);

  /* Set local parameters. */
  NumberOfDatapoints = pSetupParams->datapointCount;
  
  /* Configure timer. */
  TIM_PrescalerConfig(VOLTAMMETRY_CORE_TIMER, tim_prescaler, TIM_PSCReloadMode_Immediate);
  TIM_SetAutoreload(VOLTAMMETRY_CORE_TIMER, tim_reload);
  
  /* Tick counter initialized from negative value. This is due to apply equilibrium
    period naturally. */
  TickCounterReset = -(int32_t)((pSetupParams->equilibriumPeriod / tick_period) + 0.5f);
  DownsamplingFilterCoefficient = 1.0f / ((float)(DOWNSAMPLING_FILTER_MAGIC_NUMBER * DownsamplingNumber));
  
  // Set delegates and interfaces.
  NewDatapointDelegate = pSetupParams->newDatapointDelegate;
  MeasurementCompletedDelegate = pSetupParams->measurementCompletedDelegate;
  GeneratorFunctionInterface = pSetupParams->generatorFunctionInterface;
  
  // There should be a generator function!
  if (!GeneratorFunctionInterface)
  {
    GeneratorFunctionInterface = defaultGeneratorFunctionImplementation;
  }
  
  *pTickPeriod = tick_period;
  
  // Set state to ready.
  State = VOLTAMMETRY_CORE_STATE_READY;
}

void VoltammetryCore_Start(void)
{
  // If not ready; throw exception.
  if (State != VOLTAMMETRY_CORE_STATE_READY)
  {
    ExceptionHandler_ThrowException(\
      "Voltammetry core Start function called when the module isn't ready.\n");
  }
  
  // Reset modified variables.
  DownsamplingFilterOutput1 = 0.0f;
  DownsamplingFilterOutput2 = 0.0f;
  DownsamplingFilterOutput3 = 0.0f;
  DownsamplingFilterOutput4 = 0.0f;
    
  TickCounter = TickCounterReset;
  NextSamplingTick = DownsamplingNumber;
  DatapointCounter = 0U;
      
  // Set initial potential.
  Board_DACSignalResetnCS();
  Board_HUBSPISend(GeneratorFunctionInterface(0.0f));

  // Trigger first conversion.
  Board_ADCTriggerConvert();
    
  // Enable timer.
  TIM_Cmd(VOLTAMMETRY_CORE_TIMER, ENABLE);
  
  // Reset events.
  Events = 0;
  
  State = VOLTAMMETRY_CORE_STATE_OPERATING;
}

/***
  * @Brief      Module task executer and event processor.
  */
void VoltammetryCore_Execute(void)
{
  uint8_t __events;
  
  // If the module isn't initialized. 
  if (State != VOLTAMMETRY_CORE_STATE_OPERATING)
  {
    return;
  }
 
  __events = Events;
  
  // If a data is sampled.
  if (__events & DATA_SAMPLED_EVENT)
  {
    if (State == VOLTAMMETRY_CORE_STATE_OPERATING)
    {
      IsLocked = TRUE;
      
      NewDatapointDelegate(DownsamplingResult);
      
      if (++DatapointCounter >= NumberOfDatapoints)
      {
        TIM_Cmd(VOLTAMMETRY_CORE_TIMER, DISABLE);
      
        // Set Signal DAC value.
        Board_DACSignalResetnCS();
        Board_HUBSPISend(VGND_DAC_CODE);
        
        // Wait until the HUB SPI finished it's process.
        while (Board_HUBSPIIsBusy());
        Board_DACSignalSetnCS();
        
        State = VOLTAMMETRY_CORE_STATE_READY;
        
        // If measurement completed delegate is set; call it.
        if (MeasurementCompletedDelegate)
        {
          MeasurementCompletedDelegate();
        }
      }
      
      IsLocked = FALSE;    
    }
    
    __events ^= DATA_SAMPLED_EVENT;
  }
  
  Events = __events;
}

/***
  * @Brief      Stops the module.
  */
void VoltammetryCore_Stop(void)
{
  // If not operating; throw exception.
  if (State != VOLTAMMETRY_CORE_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Voltammetry core Stop function called when the module isn't operating.\n");
  }
  
  TIM_Cmd(VOLTAMMETRY_CORE_TIMER, DISABLE);
      
  // Set Signal DAC value. 
  Board_DACSignalResetnCS();
  Board_HUBSPISend(VGND_DAC_CODE);
      
  // Wait until the HUB SPI finishes it's process.
  while (Board_HUBSPIIsBusy());
  Board_DACSignalSetnCS();
      
  // State to pending ready.
  State = VOLTAMMETRY_CORE_STATE_READY;
}

/***
  * @Brief      Returns state of the voltammetry core module.
  */
VoltammetryCore_State_t VoltammetryCore_GetState(void)
{
  return State;
}

/* Private function implementations-------------------------------------------*/
static uint16_t defaultGeneratorFunctionImplementation(int32_t tickValue)
{
  return VGND_DAC_CODE;
}
                                    
/***
  * @Brief      Adjusts timer parameters according to required sampling frequency 
  *             and tolerances.
  * @Param      timClockFrequency-> Clock frequency of the timer.                     
  * @Param      timMaxReload-> Maximum reload value of the timer.
  * @Param      timMaxPrescaler-> Maximum prescaler value of the timer.
  * @Param      requiredSamplingFreq-> Required sampling frequency value.
  * @Param      maxRelSamplingFreqErr-> There may be some error when adjusting timer.
  *             There is a limit for this value. This parameter dictates the maximum
  *             relative sampling frequency error.
  * @Param      pTimReload-> Pointer for the calculated reload value.
  * @Param      pTimPrescaler-> Pointer for the calculated prescaler value.
  * @Param      pDownsamplingNumber-> Downsampling number indicates the downsampling
  *             ratio. Conversion value isn't recorded every timer ISR. It has been
  *             passed from a low pass filter, then downsampled and stored. Downsampling
  *             number is always equal to a power of 2.
  * @Param      pTickPeriod->Pointer for calculated tick period.
  */
static void adjustParameters(uint32_t timClockFrequency, uint32_t timMaxReload, 
                             uint16_t timMaxPrescaler, float requiredSamplingFreq, 
                             float maxRelSamplingFreqErr, uint32_t *pTimReload,
                             uint16_t *pTimPrescaler, uint32_t *pDownsamplingNumber, 
                             float *pTickPeriod)
{
  uint32_t downsampling_number;
  uint32_t tim_min_reload;
  float sampling_frequency;    
  float relative_sampling_frequency_error;
  
  // Chose tick frequency as high as possible. 
  // Maximum sampling frequency error should be lower than maximum relative frequency error.
  for (uint16_t tim_prescaler = 0; tim_prescaler <= timMaxPrescaler; tim_prescaler++)
  {
    tim_min_reload = (timClockFrequency / (tim_prescaler + 1)) / MAX_TICK_FREQUENCY;
    
    // Search for reload values.
    for (uint32_t tim_reload = tim_min_reload;  tim_reload < timMaxReload; tim_reload++)
    {
      // Calculate required downsampling number.
      downsampling_number = (uint32_t)((timClockFrequency / ((tim_prescaler + 1) * \
                                        tim_reload * requiredSamplingFreq)) + 0.5f);
      
      if (downsampling_number == 0)
      {
        downsampling_number = 1;
      }
      
      // Back calculate the sampling frequency.
      sampling_frequency = timClockFrequency / ((tim_prescaler + 1) * tim_reload \
                                                * downsampling_number);
      
      // Calculate relative sampling frequency error.
      relative_sampling_frequency_error = fabs(sampling_frequency - requiredSamplingFreq) \
                                          / requiredSamplingFreq;
      
      // Test if it satisfies the error condition.
      if (relative_sampling_frequency_error <= maxRelSamplingFreqErr)
      {
        *pTimReload = tim_reload;
        *pTimPrescaler = tim_prescaler;
        *pDownsamplingNumber = downsampling_number;
        *pTickPeriod = ((float)(tim_reload * (tim_prescaler + 1))) / timClockFrequency;
        
        return;
      }
    }
  }
}