/**
  * @author     Onur Efe
  */

/* Include files -------------------------------------------------------------*/
#include "board.h"
#include "middlewares.h"

/* Private constants ---------------------------------------------------------*/
#define DAC_VREF                                        ((double)5.0)   // DAC Reference voltage.

#define ADC_VREF                                        ((double)5.0)   // ADC Reference voltage.

#define DAC_RESOLUTION                                  16      // 16-bit DACs are used. 

#define ADC_RESOLUTION                                  16      // 16-bit ADCs are used.

#define ADC_GAIN_CORRECTION_FACTOR                      ((double)1.048)        // Correction for ADC gain error.

#define SIGNAL_DAC_CIRCUITRY_BASE_GAIN                  ((double)0.5)   // Signal DAC circuitry base gain.

#define BIAS_DAC_CIRCUITRY_GAIN                         ((double)-1)    // Bias DAC circuitry gain.

#define DAC_BIAS_RC_TIME_CONSTANT                       20      // Bias DAC RC time constant. 

#define DAC_BIAS_STABILIZATION_FACTOR                   5       // Bias DAC stabilization factor.

#define ANALOG_POWER_STABILIZATION_DELAY                100     // Power stabilization delay for
                                                                //analog circuitry.
#define ANALOG_CONFIGURATION_STABILIZATION_DELAY        2       // Stabilization delay for 
                                                                //configuration change operations.
#define SPI_CONFIGURATION_DELAY                         2

#define ADC_DUMMY_DATA                                  0x8000

/* Private variables ---------------------------------------------------------*/
// Variables to hold feedback select sequences.
static const uint8_t FB0Select[] = {0x00, 0x00, 0x41};
static const uint8_t FB1Select[] = {0x00, 0x00, 0x82};
static const uint8_t FB2Select[] = {0x00, 0x01, 0x04};
static const uint8_t FB3Select[] = {0x00, 0x02, 0x08};
static const uint8_t FB4Select[] = {0x00, 0x04, 0x10};
static const uint8_t FB5Select[] = {0x00, 0x08, 0x20};

/* Public function implementations -------------------------------------------*/
/***
  * @Brief      Interrupt service routine for power button pressed event.
  */
void Board_PowerButtonPressedISR(void)
{
  /* Shutdown device via push button controller */
  GPIO_ResetBitsOpt(PB_CONTROLLER_nKILL_PORT, PB_CONTROLLER_nKILL_PIN);
}

/***
  * @Brief      Interrupt service routine for ADC busy pin released event.
  */
void Board_ADCBusyPinReleasedISR(void)
{
  SPI_I2S_SendDataOpt(ADC_SPI, ADC_DUMMY_DATA);
}

/**
  * @Brief      Function set power indication status leds.
  *
  * @Param      status:Led status.
  */
void Board_SetPowerStatusLeds(Board_PowerIndicationLedStatus_t status)
{
  switch (status)
  {
  case BOARD_POWER_IND_LOW:
    GPIO_ResetBitsOpt(PWR_IND_0_PORT, PWR_IND_0_PIN);
    GPIO_ResetBitsOpt(PWR_IND_1_PORT, PWR_IND_1_PIN);
    GPIO_SetBitsOpt(PWR_IND_2_PORT, PWR_IND_2_PIN);
    break;
    
  case BOARD_POWER_IND_MEDIUM:
    GPIO_SetBitsOpt(PWR_IND_0_PORT, PWR_IND_0_PIN);
    GPIO_ResetBitsOpt(PWR_IND_1_PORT, PWR_IND_1_PIN);
    GPIO_ResetBitsOpt(PWR_IND_2_PORT, PWR_IND_2_PIN);
    break;
    
  case BOARD_POWER_IND_HIGH:
    GPIO_ResetBitsOpt(PWR_IND_0_PORT, PWR_IND_0_PIN);
    GPIO_SetBitsOpt(PWR_IND_1_PORT, PWR_IND_1_PIN);
    GPIO_ResetBitsOpt(PWR_IND_2_PORT, PWR_IND_2_PIN);
    break;
  }
}

/***
  * @Brief      Turns on the analog circuitry.
  */
void Board_TurnOnAnalog(void)
{
  GPIO_SetBitsOpt(ANALOG_ON_OFF_PORT, ANALOG_ON_OFF_PIN);
  
  /* Wait for power line stabilization. */
  Utils_DelayMs(ANALOG_POWER_STABILIZATION_DELAY);
}

/***
  * @Brief      Turns off the analog circuitry.
  */
void Board_TurnOffAnalog(void)
{
  GPIO_ResetBitsOpt(ANALOG_ON_OFF_PORT, ANALOG_ON_OFF_PIN);
  
  /* Wait for power line stabilization. */
  Utils_DelayMs(ANALOG_POWER_STABILIZATION_DELAY);
}

/***
  * @Brief      Configures board either of the states.
  *
  * @Param      configuration: Board configuration.
  */
void Board_ConfigureCore(Board_CoreConfiguration_t configuration)
{
  switch (configuration)
  {
  default:
  case BOARD_CORE_CONFIGURATION_OFF:
    GPIO_ResetBitsOpt(AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PORT, AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PIN);
    GPIO_ResetBitsOpt(AS_OUTPUT_STAGE_MODE_EIS_PORT, AS_OUTPUT_STAGE_MODE_EIS_PIN);
    GPIO_ResetBitsOpt(AS_OUTPUT_STAGE_EN_OUT_PORT, AS_OUTPUT_STAGE_EN_OUT_PIN);
    break;
     
  case BOARD_CORE_CONFIGURATION_VOLTAMMETRY:
    GPIO_SetBitsOpt(AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PORT, AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PIN);
    GPIO_ResetBitsOpt(AS_OUTPUT_STAGE_MODE_EIS_PORT, AS_OUTPUT_STAGE_MODE_EIS_PIN);
    GPIO_SetBitsOpt(AS_OUTPUT_STAGE_EN_OUT_PORT, AS_OUTPUT_STAGE_EN_OUT_PIN);
    break;
    
  case BOARD_CORE_CONFIGURATION_EIS:
    GPIO_ResetBitsOpt(AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PORT, AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PIN);
    GPIO_SetBitsOpt(AS_OUTPUT_STAGE_MODE_EIS_PORT, AS_OUTPUT_STAGE_MODE_EIS_PIN);
    GPIO_SetBitsOpt(AS_OUTPUT_STAGE_EN_OUT_PORT, AS_OUTPUT_STAGE_EN_OUT_PIN);
    break;
  }
}

/***
  * @Brief      Sets binary signal scaling by controlling analog switches.
  *
  * @Param      binaryScaling: Binary scaling enum. 
  */
void Board_SetBinarySignalScaling(Board_BinarySignalScaling_t binaryScaling)
{
  switch (binaryScaling)
  {
  case BOARD_BINARY_SIGNAL_SCALING_1_4:
    GPIO_ResetBitsOpt(AS_SCALER_BIN_LSB_PORT, AS_SCALER_BIN_LSB_PIN);
    GPIO_ResetBitsOpt(AS_SCALER_BIN_MSB_PORT, AS_SCALER_BIN_MSB_PIN);
    break;
    
  case BOARD_BINARY_SIGNAL_SCALING_2_4:
    GPIO_SetBitsOpt(AS_SCALER_BIN_LSB_PORT, AS_SCALER_BIN_LSB_PIN);
    GPIO_ResetBitsOpt(AS_SCALER_BIN_MSB_PORT, AS_SCALER_BIN_MSB_PIN);
    break;
    
  case BOARD_BINARY_SIGNAL_SCALING_3_4:
    GPIO_ResetBitsOpt(AS_SCALER_BIN_LSB_PORT, AS_SCALER_BIN_LSB_PIN);
    GPIO_SetBitsOpt(AS_SCALER_BIN_MSB_PORT, AS_SCALER_BIN_MSB_PIN);
    break;
    
  case BOARD_BINARY_SIGNAL_SCALING_4_4:
    GPIO_SetBitsOpt(AS_SCALER_BIN_LSB_PORT, AS_SCALER_BIN_LSB_PIN);
    GPIO_SetBitsOpt(AS_SCALER_BIN_MSB_PORT, AS_SCALER_BIN_MSB_PIN);
    break;
  }
  
  Utils_DelayMs(ANALOG_CONFIGURATION_STABILIZATION_DELAY);
}

/***
  * @Brief      Sets decimal signal scaling by controlling analog switches.
  *
  * @Param      decimalScaling: Decimal scaling enum. 
  */
void Board_SetDecimalSignalScaling(Board_DecimalSignalScaling_t decimalScaling)
{
  switch (decimalScaling)
  {
  case BOARD_DECIMAL_SIGNAL_SCALING_1_100:
    GPIO_ResetBitsOpt(AS_SCALER_DEC_LSB_PORT, AS_SCALER_DEC_LSB_PIN);
    GPIO_ResetBitsOpt(AS_SCALER_DEC_MSB_PORT, AS_SCALER_DEC_MSB_PIN);
    break;
    
  case BOARD_DECIMAL_SIGNAL_SCALING_1_10:
    GPIO_SetBitsOpt(AS_SCALER_DEC_LSB_PORT, AS_SCALER_DEC_LSB_PIN);
    GPIO_ResetBitsOpt(AS_SCALER_DEC_MSB_PORT, AS_SCALER_DEC_MSB_PIN);
    break;
    
  case BOARD_DECIMAL_SIGNAL_SCALING_1_1:
    GPIO_SetBitsOpt(AS_SCALER_DEC_LSB_PORT, AS_SCALER_DEC_LSB_PIN);
    GPIO_SetBitsOpt(AS_SCALER_DEC_MSB_PORT, AS_SCALER_DEC_MSB_PIN);
    break;
  }
  
  Utils_DelayMs(ANALOG_CONFIGURATION_STABILIZATION_DELAY);
}

/***
  * @Brief      Sets calibration relay state.
  *
  * @Param      relayState: State enumaration for the relay.
  */
void Board_SetCalibrationRelayState(Board_CalibrationRelayState_t 
                                    relayState)
{
  switch (relayState)
  {
  case BOARD_CALIBRATION_RELAY_STATE_OFF:
    GPIO_ResetBitsOpt(CALIBRATION_RELAY_PORT, CALIBRATION_RELAY_PIN);
    break;
    
  case BOARD_CALIBRATION_RELAY_STATE_ON:
    GPIO_SetBitsOpt(CALIBRATION_RELAY_PORT, CALIBRATION_RELAY_PIN);
    break;
  }
  
  /* Wait for a while. Because relay response time is relatively high.*/
  Utils_DelayMs(ANALOG_CONFIGURATION_STABILIZATION_DELAY);
}

/***
  * @Brief      Configures HUB SPI for the given channel.
  *         
  * @Param      channelID:ID of the channel configuration.
  */
void Board_HUBSPIConfigure(Board_HUBChannelID_t channelID)
{
  SPI_InitTypeDef spiInitStruct;
  
  SPI_I2S_DeInit(HUB_SPI);                              // Deinitiliaze HUB SPI.
  
  switch (channelID)                                    // Choose initialization parameters.
  {
  case BOARD_HUB_CHANNEL_ID_TIA:
    {
      spiInitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
      spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
      spiInitStruct.SPI_Mode = SPI_Mode_Master;
      spiInitStruct.SPI_CRCPolynomial = 0;
      spiInitStruct.SPI_CPHA = SPI_CPHA_2Edge;
      spiInitStruct.SPI_CPOL = SPI_CPOL_Low;
      spiInitStruct.SPI_DataSize = SPI_DataSize_8b;
      spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
      spiInitStruct.SPI_NSS = SPI_NSS_Soft;
    }
    break;
    
  case BOARD_HUB_CHANNEL_ID_DAC_BIAS:
    {
      spiInitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
      spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
      spiInitStruct.SPI_Mode = SPI_Mode_Master;
      spiInitStruct.SPI_CRCPolynomial = 0;
      spiInitStruct.SPI_CPHA = SPI_CPHA_2Edge;
      spiInitStruct.SPI_CPOL = SPI_CPOL_High;
      spiInitStruct.SPI_DataSize = SPI_DataSize_16b;
      spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
      spiInitStruct.SPI_NSS = SPI_NSS_Soft;
    }
    break;
    
  case BOARD_HUB_CHANNEL_ID_DAC_SIGNAL:
    {
      spiInitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
      spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
      spiInitStruct.SPI_Mode = SPI_Mode_Master;
      spiInitStruct.SPI_CRCPolynomial = 0;
      spiInitStruct.SPI_CPHA = SPI_CPHA_2Edge;
      spiInitStruct.SPI_CPOL = SPI_CPOL_High;
      spiInitStruct.SPI_DataSize = SPI_DataSize_16b;
      spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
      spiInitStruct.SPI_NSS = SPI_NSS_Soft;
    }
    break;
  }
  
  SPI_Init(HUB_SPI, &spiInitStruct);
  
  Utils_DelayMs(SPI_CONFIGURATION_DELAY);               // Wait for some time to enusure that the bus                          
}                                                       //is stabilized.

/***
  * @Brief      Selects transimpedance amplifier's feedback path.
  *
  * @Param      FBPath:Feedback path to be selected.
  */
void Board_TIASelectFBPath(Board_TIAFBPath_t FBPath)
{
  uint8_t buffer[3];
  
  /* Set buffer according to feedback path. */
  switch (FBPath)                                       
  {
  case BOARD_TIA_FB_PATH_0:
    buffer[2] = FB0Select[2];
    buffer[1] = FB0Select[1];
    buffer[0] = FB0Select[0];
    break;
    
  case BOARD_TIA_FB_PATH_1:
    buffer[2] = FB1Select[2];
    buffer[1] = FB1Select[1];
    buffer[0] = FB1Select[0];
    break;
    
  case BOARD_TIA_FB_PATH_2:
    buffer[2] = FB2Select[2];
    buffer[1] = FB2Select[1];
    buffer[0] = FB2Select[0];
    break;
    
  case BOARD_TIA_FB_PATH_3:
    buffer[2] = FB3Select[2];
    buffer[1] = FB3Select[1];
    buffer[0] = FB3Select[0];
    break;
    
  case BOARD_TIA_FB_PATH_4:
    buffer[2] = FB4Select[2];
    buffer[1] = FB4Select[1];
    buffer[0] = FB4Select[0];
    break;
    
  case BOARD_TIA_FB_PATH_5:
    buffer[2] = FB5Select[2];
    buffer[1] = FB5Select[1];
    buffer[0] = FB5Select[0];
    break;
  }
  
  /* Send elements of the buffer. */
  for (uint8_t i = 0; i < 3; i++)
  {
    SPI_I2S_SendDataOpt(HUB_SPI, buffer[i]);
    
    while (SPI_I2S_GetFlagStatusOpt(HUB_SPI, SPI_I2S_FLAG_TXE) == 0);
  }
  
  /* Wait till not busy. */
  while (SPI_I2S_GetFlagStatusOpt(HUB_SPI, SPI_I2S_FLAG_BSY) != 0);
  
  Utils_DelayMs(ANALOG_CONFIGURATION_STABILIZATION_DELAY);
}

/***
  * @Brief      Returns resistance of the given feedback path in units of megaohms.
  *
  * @Param      FBPath: Feedback path.
  *
  * @Return     Resistance value.
  */
double Board_TIAGetFBResistor(Board_TIAFBPath_t FBPath)
{
  float feedback_res;
  
  switch (FBPath)
  {
  case BOARD_TIA_FB_PATH_0:
    feedback_res = 0.002;
    break;
    
  case BOARD_TIA_FB_PATH_1:
    feedback_res = 0.02;
    break;
    
  case BOARD_TIA_FB_PATH_2:
    feedback_res = 0.2;
    break;
    
  case BOARD_TIA_FB_PATH_3:
    feedback_res = 2.0;
    break;
    
  case BOARD_TIA_FB_PATH_4:
    feedback_res = 20.0;
    break;
    
  case BOARD_TIA_FB_PATH_5:
    feedback_res = 200.0;
    break;
  }

  return feedback_res;
}

/***
  * @Brief      Returns Bias DAC 1LSB potential.
  *
  * @Return     Bias 1LSB potential.
  */
double Board_GetBiasDAC1LSBAppliedPotential(void)
{
  return (-BIAS_DAC_CIRCUITRY_GAIN * (DAC_VREF / (1 << DAC_RESOLUTION)));
}

/***
  * @Brief      Returns Signal DAC 1 LSB potential. 
  *
  * @Param      binarySignalScaling: Binary scaling enum.
  * @Param      decimalSignalScaling: Decimal scaling enum.
  *
  * @Return     1 LSB potential.
  */
double Board_GetSignalDAC1LSBAppliedPotential(Board_BinarySignalScaling_t binarySignalScaling,
                                              Board_DecimalSignalScaling_t decimalSignalScaling)
{
  double gain = SIGNAL_DAC_CIRCUITRY_BASE_GAIN;
 
  switch (binarySignalScaling)
  {
  default:
  case BOARD_BINARY_SIGNAL_SCALING_1_4:
    gain *= 0.25;
    break;
    
  case BOARD_BINARY_SIGNAL_SCALING_2_4:
    gain *= 0.5;
    break;
    
  case BOARD_BINARY_SIGNAL_SCALING_3_4:
    gain *= 0.75;
    break;
    
  case BOARD_BINARY_SIGNAL_SCALING_4_4:
    gain *= 1.0;
    break;
  }
  
  switch (decimalSignalScaling)
  {
  case BOARD_DECIMAL_SIGNAL_SCALING_1_100:
    gain *= 0.01;
    break;
    
  case BOARD_DECIMAL_SIGNAL_SCALING_1_10:
    gain *= 0.1;
    break;
    
  default:
  case BOARD_DECIMAL_SIGNAL_SCALING_1_1:
    gain *= 1.0;
    break;
  }
  
  return (-gain * (DAC_VREF / (1 << DAC_RESOLUTION)));
}

/***
  * @Brief      Returns ADC 1 LSB current. 
  *
  * @Param      FBPath: Feedback path enum.
  *
  * @Return     1 LSB current in units of microamps.
  */
double Board_GetADC1LSBCurrent(Board_TIAFBPath_t FBPath)
{
  double gain;  // Volts/Microamps.
  
  switch (FBPath)
  {
  default:
  case BOARD_TIA_FB_PATH_0:
    gain = 0.002 * ADC_GAIN_CORRECTION_FACTOR;
    break;
    
  case BOARD_TIA_FB_PATH_1:
    gain = 0.02 * ADC_GAIN_CORRECTION_FACTOR;
    break;
    
  case BOARD_TIA_FB_PATH_2:
    gain = 0.2 * ADC_GAIN_CORRECTION_FACTOR;
    break;
    
  case BOARD_TIA_FB_PATH_3:
    gain = 2.0 * ADC_GAIN_CORRECTION_FACTOR;
    break;
    
  case BOARD_TIA_FB_PATH_4:
    gain = 20.0 * ADC_GAIN_CORRECTION_FACTOR;
    break;
    
  case BOARD_TIA_FB_PATH_5:
    gain = 200.0 * ADC_GAIN_CORRECTION_FACTOR;
    break;
  }
  
  return (ADC_VREF / ((1 << ADC_RESOLUTION) * gain));
}

/***
  * @Brief      Returns DAC Bias stabilization period in units of miliseconds.
  *
  * @Return     Period in ms.
  */
uint32_t Board_GetDACBiasStabilizationPeriod(void)
{
  return (DAC_BIAS_RC_TIME_CONSTANT * DAC_BIAS_STABILIZATION_FACTOR);
}

/***
  * @Brief      Checks if the HUB SPI is busy.
  *
  * @Return     TRUE or FALSE.
  */
uint8_t Board_HUBSPIIsBusy(void)
{
  return ((SPI_I2S_GetFlagStatusOpt(HUB_SPI, SPI_I2S_FLAG_TXE) == 0) || \
          (SPI_I2S_GetFlagStatusOpt(HUB_SPI, SPI_I2S_FLAG_BSY) != 0));
}