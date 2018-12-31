/**
  * @author Onur Efe
  */

#ifndef __BOARD_H
#define __BOARD_H

#include "generic.h"
#include "stm32f4xx_conf.h"
#include "peripheral_mapping.h"
#include "board_bt_module.h"

/* Exported constants --------------------------------------------------------*/
#define BOARD_DAC_VIRTUAL_GROUND_CODE           ((uint16_t)((MAX_UINT16 + 1) / 2))
#define BOARD_CALIB_ELEMENT_IMP_REAL            ((double)15.0)
#define BOARD_CALIB_ELEMENT_IMP_IMG             ((double)0.0)

#define BOARD_MAX_BIAS_POTENTIAL                1.0f
#define BOARD_MIN_BIAS_POTENTIAL                -1.0f
#define BOARD_MAX_SIGNAL_POTENTIAL              1.0f
#define BOARD_MIN_SIGNAL_POTENTIAL              -1.0f
    
/* Exported types ------------------------------------------------------------*/
typedef enum
{
  BOARD_POWER_IND_LOW = 0x00,
  BOARD_POWER_IND_MEDIUM,
  BOARD_POWER_IND_HIGH
} Board_PowerIndicationLedStatus_t;

typedef enum
{
  BOARD_CORE_CONFIGURATION_OFF = 0x00,
  BOARD_CORE_CONFIGURATION_VOLTAMMETRY,
  BOARD_CORE_CONFIGURATION_EIS
} Board_CoreConfiguration_t;

typedef enum
{
  BOARD_BINARY_SIGNAL_SCALING_1_4 = 0x00,
  BOARD_BINARY_SIGNAL_SCALING_2_4,
  BOARD_BINARY_SIGNAL_SCALING_3_4,
  BOARD_BINARY_SIGNAL_SCALING_4_4
} Board_BinarySignalScaling_t;

typedef enum
{
  BOARD_DECIMAL_SIGNAL_SCALING_1_100 = 0x00,
  BOARD_DECIMAL_SIGNAL_SCALING_1_10,
  BOARD_DECIMAL_SIGNAL_SCALING_1_1
} Board_DecimalSignalScaling_t;

typedef enum
{
  BOARD_CALIBRATION_RELAY_STATE_OFF = 0x00,
  BOARD_CALIBRATION_RELAY_STATE_ON
} Board_CalibrationRelayState_t;

typedef enum
{
  BOARD_HUB_CHANNEL_ID_TIA = 0x00,
  BOARD_HUB_CHANNEL_ID_DAC_BIAS,
  BOARD_HUB_CHANNEL_ID_DAC_SIGNAL
} Board_HUBChannelID_t;

typedef enum
{
  BOARD_TIA_FB_PATH_0 = 0x00,
  BOARD_TIA_FB_PATH_1,
  BOARD_TIA_FB_PATH_2,
  BOARD_TIA_FB_PATH_3,
  BOARD_TIA_FB_PATH_4,
  BOARD_TIA_FB_PATH_5
} Board_TIAFBPath_t;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Turns on the analog circuitry.
  */
extern void Board_TurnOnAnalog(void);

/***
  * @Brief      Turns off the analog circuitry.
  */
extern void Board_TurnOffAnalog(void);

/**
  * @Brief      Function set power indication status leds.
  *
  * @Param      status:Led status.
  */
extern void Board_SetPowerStatusLeds(Board_PowerIndicationLedStatus_t status);

/***
  * @Brief      Configures board either of the states.
  *
  * @Param      configuration: Board configuration.
  */
extern void Board_ConfigureCore(Board_CoreConfiguration_t configuration);

/***
  * @Brief      Sets binary signal scaling by controlling analog switches.
  *
  * @Param      binaryScaling: Binary scaling enum. 
  */
extern void Board_SetBinarySignalScaling(Board_BinarySignalScaling_t binaryScaling);

  /***
  * @Brief      Sets decimal signal scaling by controlling analog switches.
  *
  * @Param      decimalScaling: Decimal scaling enum. 
  */
extern void Board_SetDecimalSignalScaling(Board_DecimalSignalScaling_t decimalScaling);

/***
  * @Brief      Sets calibration relay state.
  *
  * @Param      relayState: State enumaration for the relay.
  */
extern void Board_SetCalibrationRelayState(Board_CalibrationRelayState_t 
                                           relayState);

/***
  * @Brief      Configures HUB SPI for the given channel.
  *         
  * @Param      channelID:ID of the channel configuration.
  */
extern void Board_HUBSPIConfigure(Board_HUBChannelID_t channelID);

/***
  * @Brief      Selects transimpedance amplifier's feedback path.
  *
  * @Param      FBPath:Feedback path to be selected.
  */
extern void Board_TIASelectFBPath(Board_TIAFBPath_t FBPath);

/***
  * @Brief      Returns resistance of the given feedback path in units of megaohms.
  *
  * @Param      FBPath: Feedback path.
  *
  * @Return     Resistance value.
  */
extern double Board_TIAGetFBResistor(Board_TIAFBPath_t FBPath);

/***
  * @Brief      Returns Bias DAC 1LSB potential.
  *
  * @Return     Bias 1LSB potential.
  */
extern double Board_GetBiasDAC1LSBAppliedPotential(void);

/***
  * @Brief      Returns Signal DAC 1 LSB potential. 
  *
  * @Param      binarySignalScaling: Binary scaling enum.
  * @Param      decimalSignalScaling: Decimal scaling enum.
  *
  * @Return     1 LSB potential.
  */
extern double Board_GetSignalDAC1LSBAppliedPotential(Board_BinarySignalScaling_t binarySignalScaling,
                                                     Board_DecimalSignalScaling_t decimalSignalScaling);

/***
  * @Brief      Returns ADC 1 LSB current. 
  *
  * @Param      FBPath: Feedback path enum.
  *
  * @Return     1 LSB current in units of microamps.
  */
extern double Board_GetADC1LSBCurrent(Board_TIAFBPath_t FBPath);


/***
  * @Brief      Returns DAC Bias stabilization period in units of miliseconds.
  *
  * @Return     Period in ms.
  */
extern uint32_t Board_GetDACBiasStabilizationPeriod(void);

/***
  * @Brief      Checks if the HUB SPI is busy.
  *
  * @Return     TRUE or FALSE.
  */
extern uint8_t Board_HUBSPIIsBusy(void);

/* Static inline functions ---------------------------------------------------*/
/***
  * @Brief      Enables HUB SPI.
  */
__STATIC_INLINE void Board_HUBSPIEnable(void)
{
  SPI_Cmd(HUB_SPI, ENABLE);
}

/***
  * @Brief      Disables HUB SPI.
  */
__STATIC_INLINE void Board_HUBSPIDisable(void)
{
  SPI_Cmd(HUB_SPI, DISABLE);
}

/***
  * @Brief      Sends value over the HUB SPI.
  */
__STATIC_INLINE void Board_HUBSPISend(uint16_t value)
{
  SPI_I2S_SendDataOpt(HUB_SPI, value);
}

/***
  * @Brief      Sets TIA nCS pin.
  */
__STATIC_INLINE void Board_TIASetnCS(void)
{
  GPIO_SetBitsOpt(TIA_nCS_PORT, TIA_nCS_PIN);
}

/***
  * @Brief      Resets TIA nCS pin.
  */
__STATIC_INLINE void Board_TIAResetnCS(void)
{
  GPIO_ResetBitsOpt(TIA_nCS_PORT, TIA_nCS_PIN);
}

/***
  * @Brief      Sets TIA nLATCH pin.
  */
__STATIC_INLINE void Board_TIASetnLATCH(void)
{
  GPIO_SetBitsOpt(TIA_nLATCH_PORT, TIA_nLATCH_PIN);
}

/***
  * @Brief      Resets TIA nLATCH pin.
  */
__STATIC_INLINE void Board_TIAResetnLATCH(void)
{
  GPIO_ResetBitsOpt(TIA_nLATCH_PORT, TIA_nLATCH_PIN);
}

/***
  * @Brief      Resets DAC Bias nCS pin.
  */
__STATIC_INLINE void Board_DACBiasResetnCS(void)
{
  GPIO_ResetBitsOpt(DAC_BIAS_nCS_PORT, DAC_BIAS_nCS_PIN);
}

/***
  * @Brief      Sets DAC Bias nCS pin.
  */
__STATIC_INLINE void Board_DACBiasSetnCS(void)
{
  GPIO_SetBitsOpt(DAC_BIAS_nCS_PORT, DAC_BIAS_nCS_PIN);
}

/***
  * @Brief      Resets DAC Bias nLDAC pin.
  */
__STATIC_INLINE void Board_DACBiasResetnLDAC(void)
{
  GPIO_ResetBitsOpt(DAC_BIAS_nLDAC_PORT, DAC_BIAS_nLDAC_PIN);
}

/***
  * @Brief      Sets DAC Bias nLDAC pin.
  */
__STATIC_INLINE void Board_DACBiasSetnLDAC(void)
{
  GPIO_SetBitsOpt(DAC_BIAS_nLDAC_PORT, DAC_BIAS_nLDAC_PIN);
}

/***
  * @Brief      Resets DAC Signal nCS pin.
  */
__STATIC_INLINE void Board_DACSignalResetnCS(void)
{
  GPIO_ResetBitsOpt(DAC_SIGNAL_nCS_PORT, DAC_SIGNAL_nCS_PIN);
}

/***
  * @Brief      Sets DAC Signal nCS pin.
  */
__STATIC_INLINE void Board_DACSignalSetnCS(void)
{
  GPIO_SetBitsOpt(DAC_SIGNAL_nCS_PORT, DAC_SIGNAL_nCS_PIN);
}

/***
  * @Brief      Resets DAC Signal nLDAC pin.
  */
__STATIC_INLINE void Board_DACSignalResetnLDAC(void)
{
  GPIO_ResetBitsOpt(DAC_SIGNAL_nLDAC_PORT, DAC_SIGNAL_nLDAC_PIN);
}

/***
  * @Brief      Sets DAC Signal nLDAC pin.
  */
__STATIC_INLINE void Board_DACSignalSetnLDAC(void)
{
  GPIO_SetBitsOpt(DAC_SIGNAL_nLDAC_PORT, DAC_SIGNAL_nLDAC_PIN);
}

/***
  * @Brief      Enable ADC SPI.
  */
__STATIC_INLINE void Board_ADCSPIEnable(void)
{
  SPI_Cmd(ADC_SPI, ENABLE);
}

/***
  * @Brief      Disable ADC SPI.
  */
__STATIC_INLINE void Board_ADCSPIDisable(void)
{
  SPI_Cmd(ADC_SPI, DISABLE);
}

/***
  * @Brief      Gets ADC value from the ADC_SPI.
  */
__STATIC_INLINE int16_t Board_ADCGetValue(void)
{
  return ((int16_t)SPI_I2S_ReceiveDataOpt(ADC_SPI));
}

/***
  * @Brief      Triggers ADC conversion.
  */
__STATIC_INLINE void Board_ADCTriggerConvert(void)
{
  GPIO_SetBitsOpt(ADC_CNV_PORT, ADC_CNV_PIN);
  GPIO_ResetBitsOpt(ADC_CNV_PORT, ADC_CNV_PIN);
}
#endif