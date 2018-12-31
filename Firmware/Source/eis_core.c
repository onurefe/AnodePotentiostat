/**
  * @author     Onur Efe
  */

#include <math.h>
#include "board.h"
#include "eis_core.h"
#include "middlewares.h"
#include "generic.h"
    
/* Prequisities --------------------------------------------------------------*/
/*
  This module is critical in some ways. Overhead to the EISCore_MicroSchedulerTickISR
  function should be minimum. Other way, the program will crash, because of the
  interrupt function operation shouldn't be completed. So, the measurement results will
  be errorenous. Such an error wouldn't be detected because a detection mechanism 
  also would cause an overhead.
  Besides this, microscheduler timer frequency and microscheduler prescaler range
  is defined in this module. Any timer shouldn't be used by any other module. 
*/

/* Private definitions -------------------------------------------------------*/
#define VGND_DAC_CODE_FL                        ((float)(MAX_UINT16 + 1) / 2)
#define VGND_DAC_CODE                           ((MAX_UINT16 + 1) / 2)

#define TICK_FREQUENCY                          ((double)400000.0)
#define TICK_PERIOD                             ((double)1 / TICK_FREQUENCY)
#define TIM_PRESCALER                           0U
#define TIM_RELOAD                              210
    
#define SUBSAMPLING_NUMBER                      65536            // Should be even number.
#define SUBSAMPLING_MASK                        0x8000FFFF                    

#define START_EVENT                             0x01
#define SAMPLING_EVENT                          0x02
#define STOP_EVENT                              0x04

/* Private function prototypes -----------------------------------------------*/
static inline void rotatePhasor(float *pX, float *pY, float xr, float yr);

/* Private variables ---------------------------------------------------------*/
/* Measurement state. */
static EISCore_State_t  State = EIS_CORE_STATE_UNINIT;
static uint8_t          Events;
static uint8_t          IsLocked = FALSE;

/* Store variables. */
EISCore_MeasurementCompletedDelegate_t  MeasurementCompletedDelegate;

static uint32_t         LeapTicks;
static uint32_t         NumOfSubSamples;
static float            AmplitudeCoeff;

static float            PhasorBaseXStart;
static float            PhasorBaseYStart;

/* Modified variables. */
static volatile float   ConversionValue;

static float            PhasorX, PhasorY;
static float            PhasorBaseX, PhasorBaseY;

static float            MicroRotX, MicroRotY;
static float            MacroRotX, MacroRotY;

static float            SubSumX, SubSumY;
static volatile float   SampleX, SampleY;
static double           SumX, SumY;

static uint32_t         TickCounter;
static uint32_t         SubSampleCounter;


/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      ISR routine for timer interrupt. Should be called with minimum 
  *             overhead. 
  */
void EISCore_TimerTickISR(void)
{
  if ((State != EIS_CORE_STATE_OPERATING) || (IsLocked == TRUE))
  {
    return;
  }
    
  Board_DACSignalSetnCS();
  
  /* Get last conversion result. */
  ConversionValue = (float)Board_ADCGetValue();

  SubSumX += (ConversionValue * PhasorX);
  SubSumY += (ConversionValue * PhasorY);
  
  Board_DACSignalResetnCS();            // Frame signal DAC data.
  
  // Generate DAC code and set it to DAC.
  SPI_I2S_SendDataOpt(HUB_SPI, ((uint16_t)(VGND_DAC_CODE_FL + PhasorY * AmplitudeCoeff)));
  
  // Trigger next conversion.
  Board_ADCTriggerConvert();
  
  // Rotate phasor.
  rotatePhasor(&PhasorX, &PhasorY, MicroRotX, MicroRotY);
  
  /* Check for subsampling instant */
  if ((++TickCounter & SUBSAMPLING_MASK) == 0)
  {
    SampleX = SubSumX;
    SampleY = SubSumY;
    
    PhasorX = PhasorBaseX;
    PhasorY = PhasorBaseY;
    
    SubSumX = 0.0f;
    SubSumY = 0.0f;
    
    Events |= SAMPLING_EVENT;                   // Set sampling event.
  }
}

/***
  * @Brief      Setup function of the EIS core. Function should called before any
  *             other operation.
  *
  * @Param      pMeasParams->Pointer to measurement params struct.
  */
void EISCore_Setup(EISCore_MeasParams_t *pMeasParams)
{
  double total_measurement_time;
  double subsampling_period = (((double)SUBSAMPLING_NUMBER) / TICK_FREQUENCY);
  
  /* Check state. */
  if (State == EIS_CORE_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "EIS core module setup function called when the module is operating.\n");
  }
  
  // Calculate total measurement time.
  total_measurement_time = ((double)pMeasParams->cycles) / pMeasParams->frequency;
  
  // Calculate leap ticks.
  LeapTicks = (uint32_t)(TICK_FREQUENCY * \
    fmod(total_measurement_time, subsampling_period));
  
  // Make leap ticks even.
  if ((LeapTicks % 2) != 0)
  {
    LeapTicks++;
  }
  
  /* Calculate number of sub samples. */
  NumOfSubSamples = (uint32_t)(total_measurement_time / subsampling_period);
  
  /* Set phasors. */
  {
    double norm_freq;
    double small_angle;
    double big_angle;
  
    norm_freq = pMeasParams->frequency / TICK_FREQUENCY;
    small_angle = 2.0 * M_PI * norm_freq;
    big_angle = small_angle * SUBSAMPLING_NUMBER;
      
    /* Calculate phasors. */
    MicroRotX = cos(small_angle);
    MicroRotY = sin(small_angle);
    MacroRotX = cos(big_angle);
    MacroRotY = sin(big_angle);
    
    /* Patch leap ticks to initial subsampling period. */
    PhasorBaseXStart = cos(small_angle * LeapTicks);
    PhasorBaseYStart = sin(small_angle * LeapTicks);
  }
  
  MeasurementCompletedDelegate = pMeasParams->measurementCompletedDelegate;
  AmplitudeCoeff = ((float)pMeasParams->signal_amp_pp_dac_code) / 2;
    
  /* Configure timer. */
  TIM_PrescalerConfig(EIS_CORE_TIMER, TIM_PRESCALER, TIM_PSCReloadMode_Immediate);
  TIM_SetAutoreload(EIS_CORE_TIMER, TIM_RELOAD);
  
  Events = 0;
  
  // Set state to ready.
  State = EIS_CORE_STATE_READY;
}

/***
  * @Brief      Sets start event. 
  */
void EISCore_Start(void)
{
  Events |= START_EVENT;
}

/***
  * @Brief      EIS Core module's task executer. Should be called periodically.
  */
void EISCore_Execute(void)
{
  uint8_t __events;
  
  if (State == EIS_CORE_STATE_UNINIT)
  {
    ExceptionHandler_ThrowException(\
      "EIS core executer called when the module isn't initialized.\n");
  }
                                    
  /* Events register is sampled due to prevent race condition over events register.
    There is no need to sample state register, because it can't be changed outside
    of the execute function. */
  __events = Events;
  
  /* Sampling event occurred. */
  if (__events & SAMPLING_EVENT)
  {
    if (State == EIS_CORE_STATE_OPERATING)
    {
      IsLocked = TRUE;
      
      /* Add sampled sums. */
      SumX += (double)SampleX;
      SumY += (double)SampleY;
 
      // Update Base Phasor.
      rotatePhasor(&PhasorBaseX, &PhasorBaseY, MacroRotX, MacroRotY);
      
      /* Increase subsample counter. */
      if (SubSampleCounter++ >= NumOfSubSamples)
      {
        TIM_Cmd(EIS_CORE_TIMER, DISABLE);
      
        // Set Signal DAC value.
        Board_DACSignalResetnCS();
        Board_HUBSPISend(VGND_DAC_CODE);
      
        // Wait until the HUB SPI finished it's process.
        while (Board_HUBSPIIsBusy());
        Board_DACSignalSetnCS();
      
        MeasurementCompletedDelegate();
        
        /* State to pending ready. */
        State = EIS_CORE_STATE_READY;
      }
      
      IsLocked = FALSE;
    }
    
    __events ^= SAMPLING_EVENT;
  }
  
  /* Start event occurred. */
  if (__events & START_EVENT)
  {
    if (State == EIS_CORE_STATE_READY)
    {
      IsLocked = TRUE;
      
      /* Reset variables. */
      PhasorX = 1.0f;
      PhasorY = 0.0f;
      PhasorBaseX = PhasorBaseXStart;
      PhasorBaseY = PhasorBaseYStart;
      
      SumX = 0.0f;
      SumY = 0.0f;
      SubSumX = 0.0f;
      SubSumY = 0.0f;
      
      SubSampleCounter = 0;
      TickCounter = MAX_UINT32 - LeapTicks;
  
      /* Set initial potential. */
      Board_DACSignalResetnCS();
      Board_HUBSPISend(VGND_DAC_CODE);
      
      // Trigger first conversion.
      Board_ADCTriggerConvert();
      
      /* Reset Timer. */
      TIM_SetCounter(EIS_CORE_TIMER, 0U);
      TIM_ClearFlagOpt(EIS_CORE_TIMER, TIM_IT_Update);

      // Enable timer.
      TIM_Cmd(EIS_CORE_TIMER, ENABLE);
      
      State = EIS_CORE_STATE_OPERATING;
      
      IsLocked = FALSE;
    }
    
    __events ^= START_EVENT;
  }
  
  /* Stop event occurred. */
  if (__events & STOP_EVENT)
  {
    /* Process event if the state is operating. */
    if (State == EIS_CORE_STATE_OPERATING)
    {
      IsLocked = TRUE;
      
      TIM_Cmd(EIS_CORE_TIMER, DISABLE);
      
      // Set Signal DAC value. 
      Board_DACSignalResetnCS();
      Board_HUBSPISend(VGND_DAC_CODE);
      
      // Wait until the HUB SPI finished it's process.
      while (Board_HUBSPIIsBusy());
      Board_DACSignalSetnCS();
      
      /* State to pending ready. */
      State = EIS_CORE_STATE_READY;
      
      IsLocked = FALSE;
    }
    
    __events ^= STOP_EVENT;
  }
  
  // Update events register.
  Events = __events;
}

/***
  * @Brief      Sets stop event.
  */
void EISCore_Stop(void)
{
  Events |= STOP_EVENT;
}

/***
  * @Brief      Gets module's state.
  */
EISCore_State_t EISCore_GetState(void)
{
  return State;
}

/***
  * @Brief      Gets result.
  */
void EISCore_GetResult(double *pAverageX, double *pAverageY)
{
  uint32_t num_of_samples;
  
  num_of_samples = SUBSAMPLING_NUMBER * NumOfSubSamples + LeapTicks;
  
  /* Calculate averages. */
  *pAverageX = SumX / num_of_samples;
  *pAverageY = SumY / num_of_samples;
}

/* Private function implementations-------------------------------------------*/
static inline void rotatePhasor(float *pX, float *pY, float xr, float yr)
{
  static float temp;
  
  /* Calculate rotated phasor. */
  temp = (*pX) * xr - (*pY) * yr;
  (*pY) = (*pX) * yr + (*pY) * xr;
  (*pX) = temp;
}