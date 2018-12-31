/**
  * @author     Onur Efe
  */ 

#include "bias_adjuster.h"

/* Time constants in units of seconds. */
#define CONTROL_FREQUENCY               20000U
#define CONTROL_PERIOD                  ((float)1.0f / CONTROL_FREQUENCY)

#define INTERNAL_REFERENCE_VOLTAGE      1.21f

#define ADC_FILTER_TC                   0.025f
#define ADC_FILTER_COEFF                (CONTROL_PERIOD / ADC_FILTER_TC)

#define PID_GAIN                        1.5f
#define INTEGRAL_TC                     0.2f

/* Private function prototypes -----------------------------------------------*/

static inline void executePID(float input, float *pLastInput, double *pLastIntegratedSum, 
                              float *pOutput);

/* Private variables ---------------------------------------------------------*/

/* Module state. */
static BiasAdjuster_State_t State;

/* Required reference voltage. */
static float    RequiredRefVoltage;

/* About plant reference. */
static float    PlantRefVoltage;
static float    PlantRefVoltageFiltered;

/* About internal voltage reference and calculating VREF from it. */
static float    AdcVrefVoltage;
static float    AdcVrefVoltageFiltered;

/* PID controller static variables. */
static float    PIDLastInput;
static double   PIDLastIntegratedSum;
static float    PIDResponse;

/* Pointers for conversion results. */
static uint16_t *pChannelConversionValue;
static uint16_t *pIntRefConversionValue;

/* Start bias controller. */
void BiasAdjuster_Start(float referenceVoltage)
{
  uint16_t tim_prescaler;
  uint32_t tim_reload;
  uint32_t tim_frequency;
  uint32_t tim_max_reload;
  float prescaled_tim_frequency;
  
  /* Check for state compatibility. */
  if (State != BIAS_ADJUSTER_STATE_READY)
  {
    ExceptionHandler_ThrowException(INCOMPATIBLE_STATE);
  }
  
  /* Set static class variables. */
  PlantRefVoltage = 0.0f;
  PlantRefVoltageFiltered = 0.0f;
  
  AdcVrefVoltage = EIS_ADC_INT_RANGE;
  AdcVrefVoltageFiltered = EIS_ADC_INT_RANGE;
  
  RequiredRefVoltage = referenceVoltage;
  
  PIDLastInput = 0.0f;
  PIDLastIntegratedSum = 0.0f;
  
  pChannelConversionValue = HALAdapter_BiasAdjuster_GetChannelConversionValuePtr();
  pIntRefConversionValue = HALAdapter_BiasAdjuster_GetIntRefConversionValuePtr();
  
  Board_DACBias_Open();
  Board_DACBias_SetValue((MAX_UINT16 + 1) / 2);
  
  HALAdapter_BiasAdjuster_EnableADC();
  
  HALAdapter_BiasAdjuster_GetTIMProperties(&tim_max_reload, &tim_frequency);
  
  /* Calculate timer prescaler and reload values. */
  tim_prescaler = (uint16_t)(((float)tim_frequency) / tim_max_reload);
  prescaled_tim_frequency = ((float)tim_frequency) / (tim_prescaler + 1); 
  tim_reload = (uint32_t)(prescaled_tim_frequency / CONTROL_FREQUENCY);
  
  HALAdapter_BiasAdjuster_ConfigTIM(tim_prescaler, tim_reload);
  
  HALAdapter_BiasAdjuster_ADCStartConversion(); // Start analog-digital conversion. 
  
  HALAdapter_BiasAdjuster_EnableTIM();          // Enable timer.

  State = BIAS_ADJUSTER_STATE_OPERATING;        // Set state to operating.
}

void BiasAdjuster_Stop(void)
{
  /* Check for state compatibility. */
  if (State != BIAS_ADJUSTER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(INCOMPATIBLE_STATE);
  }
  
  /* Disable used external objects. */
  HALAdapter_BiasAdjuster_DisableTIM();
  HALAdapter_BiasAdjuster_DisableADC();
  
  Board_DACBias_Close();
  
  State = BIAS_ADJUSTER_STATE_READY;
}

BiasAdjuster_State_t BiasAdjuster_GetState(void)
{
  return State;
}

void BiasAdjuster_TimerTickISR(void)
{
  /* Discard incompatible operation. */
  if (State != BIAS_ADJUSTER_STATE_OPERATING)
  {
    return;
  }
  
  /* Convert ADC codes to voltages. */
  PlantRefVoltage = AdcVrefVoltageFiltered * (((float) *pChannelConversionValue) / \
                    MAX_UINT12) - EIS_VIRTUAL_GND_VOLTAGE;
  
  AdcVrefVoltage = INTERNAL_REFERENCE_VOLTAGE * (MAX_UINT12 / ((float) *pIntRefConversionValue));
  
  /* Filter them. */
  PlantRefVoltageFiltered = ADC_FILTER_COEFF * PlantRefVoltage + \
                            (1.0f - ADC_FILTER_COEFF) * PlantRefVoltageFiltered;
  
  AdcVrefVoltageFiltered = ADC_FILTER_COEFF * AdcVrefVoltage + \
                            (1.0f - ADC_FILTER_COEFF) * AdcVrefVoltageFiltered;

  /* Execute PID controller. */
  executePID((RequiredRefVoltage - PlantRefVoltageFiltered), &PIDLastInput, 
             &PIDLastIntegratedSum, &PIDResponse);
  
  /* Set DAC Bias value. */
  Board_DACBias_SetValue((uint16_t)(MAX_UINT16 * (0.5f - PIDResponse * (1.0f / EIS_DAC_RANGE))));
  
  /* Start analog-digital conversion. */
  HALAdapter_BiasAdjuster_ADCStartConversion();
}

static inline void executePID(float input, float *pLastInput, double *pLastIntegratedSum, 
                              float *pOutput)
{
  float response;
  double integrated_sum;
  
  integrated_sum = *pLastIntegratedSum + (input + *pLastInput) / 2.0f;
  
  response = input;                                                             // Proportional
  response += integrated_sum / (INTEGRAL_TC * CONTROL_FREQUENCY);     // Integral
  response *= PID_GAIN;
  
  /* Limit output if the response is bigger than DAC capability. */
  if (response >= (EIS_DAC_RANGE / 2.0f))
  {
    response = EIS_DAC_RANGE / 2.0f;
  }
  
  else if (response <= (-EIS_DAC_RANGE / 2.0f))
  {
    response = EIS_DAC_RANGE / 2.0f;
  }
  
  /* Else, if the output is not saturated, update integral sum. */
  else
  {
    *pLastIntegratedSum = integrated_sum;
  }
  
  *pLastInput = input;  // Update last input.
  *pOutput = response;  // Set output.
}