#include "device_manager.h"
#include "characteristic_server.h"
#include "amperometry.h"
#include "eis.h"
#include "middlewares.h"

/* Private constants ---------------------------------------------------------*/
// Abbrevations.
#define PROPERTY_READABLE               CHARACTERISTIC_SERVER_CHAR_PROP_READABLE
#define PROPERTY_WRITABLE               CHARACTERISTIC_SERVER_CHAR_PROP_WRITABLE
#define PROPERTY_VARIABLE_LENGTH        CHARACTERISTIC_SERVER_CHAR_PROP_VARIABLE_LENGTH
#define PROPERTY_REGISTERED             CHARACTERISTIC_SERVER_CHAR_PROP_REGISTERED

// Amperometry Service characteristic IDs.
#define AMPEROMETRY_SERVICE_SAMPLING_FREQUENCY_CHAR_ID          0x0000
#define AMPEROMETRY_SERVICE_DATAPOINT_COUNT_CHAR_ID             0x0001
#define AMPEROMETRY_SERVICE_POTENTIAL_CHAR_ID                   0x0002
#define AMPEROMETRY_SERVICE_RANGE_CHAR_ID                       0x0003
#define AMPEROMETRY_SERVICE_EQUILIBRIUM_PERIOD_CHAR_ID          0x0004
#define AMPEROMETRY_SERVICE_DATAPOINT_CHAR_ID                   0x0005

// Device Control Service characteristic IDs.
#define DEV_CTRL_SERVICE_COMMAND_POINT_CHAR_ID                  0x0100
#define DEV_CTRL_SERVICE_COMMAND_RESPONSE_CHAR_ID               0x0101
#define DEV_CTRL_SERVICE_STATUS_CHAR_ID                         0x0102
#define DEV_CTRL_SERVICE_BATTERY_LEVEL_CHAR_ID                  0x0103

// Amperometry Service characteristic validations.
#define AMPEROMETRY_SERVICE_IS_VALID_SAMPLING_FREQUENCY(f) \
(((f) <= AMPEROMETRY_MAX_SAMPLING_FREQUENCY) && ((f) >= AMPEROMETRY_MIN_SAMPLING_FREQUENCY))

#define AMPEROMETRY_SERVICE_IS_VALID_DATAPOINT_COUNT(n) \
((n) != 0)

#define AMPEROMETRY_SERVICE_IS_VALID_POTENTIAL(v) \
(((v) >= BOARD_MIN_SIGNAL_POTENTIAL) && ((v) <= BOARD_MAX_SIGNAL_POTENTIAL))

#define AMPEROMETRY_SERVICE_IS_VALID_RANGE(r) \
(((r) == RANGE_1MA) || ((r) == RANGE_100UA) || ((r) == RANGE_10UA) || ((r) == RANGE_1UA) || \
 ((r) == RANGE_100NA))

#define AMPEROMETRY_SERVICE_IS_VALID_EQUILIBRIUM_PERIOD(t) \
((t) >= 0.0f)

// Device Control Service characteristic validations.
#define DEV_CTRL_SERVICE_IS_VALID_COMMAND(c) \
(((c) == COMMAND_START_AMPEROMETRY) || ((c) == COMMAND_STOP_MEASUREMENT))

/* Private typedefs ----------------------------------------------------------*/
// Short name for characteristic.
typedef CharacteristicServer_Characteristic_t        Characteristic_t;

// Enumerations for characteristics.
// Range type.
typedef enum
{
  RANGE_1MA = 0,
  RANGE_100UA,
  RANGE_10UA,
  RANGE_1UA,
  RANGE_100NA
} Range_t;

// Available commands.
typedef enum
{
  COMMAND_START_AMPEROMETRY = 0,
  COMMAND_STOP_MEASUREMENT = 1
} Command_t;

// Command responses.
typedef enum
{
  COMMAND_RESP_SUCCESS = 0,
  COMMAND_RESP_INVALID_COMMAND = 1,
  COMMAND_RESP_STATE_NOT_COMPATIBLE = 2,
  COMMAND_RESP_INVALID_SETUP_PARAMETER = 3
} CommandResp_t;

// Device status(also a control type).
typedef enum
{
  DEVICE_STATUS_IDLE = 0,
  DEVICE_STATUS_AMPEROMETRY_MEASUREMENT = 1
} DeviceStatus_t;

/* Private function declerations ---------------------------------------------*/
static void                     startAmperometry(void);
static Bool_t                   checkAmperometryParameters(void);
static Board_TIAFBPath_t        getFBPath(Range_t range);
static void                     measurementCompletedEventHandler(void);
static void                     amperometryNewDatapointEventHandler(float datapoint);
static void                     writeEventHandler(uint16_t charId);
static void                     connectionStateChangedEventHandler(Bool_t isConnected);

/* Private variables. --------------------------------------------------------*/
// Control variables.
static DeviceManager_State_t    State = DEVICE_MANAGER_STATE_UNINIT;
static DeviceStatus_t           DeviceStatus;

// Amperometry Service characteristics.
static float                    AmperometryServiceSamplingFrequencyCharData;
static uint16_t                 AmperometryServiceDatapointCountCharData;
static float                    AmperometryServicePotentialCharData;
static uint8_t                  AmperometryServiceRangeCharData;
static float                    AmperometryServiceEquilibriumPeriodCharData;
static float                    AmperometryServiceDatapointCharData;

// Device Control Service characteristics.
static Command_t                DevCtrlServiceCommandPointCharData;
static CommandResp_t            DevCtrlServiceCommandResponseCharData;
static DeviceStatus_t           DevCtrlServiceStatusCharData;
static float                    DevCtrlServiceBatteryLevelCharData;

// Characteristic Database.
static Characteristic_t CharacteristicDatabase[] = \
  {
    // Amperometry Service.
    // Sampling Frequency Characteristic
    {
      AMPEROMETRY_SERVICE_SAMPLING_FREQUENCY_CHAR_ID,
      (uint8_t *)&AmperometryServiceSamplingFrequencyCharData,
      sizeof(AmperometryServiceSamplingFrequencyCharData),
      (PROPERTY_READABLE | PROPERTY_WRITABLE)
    },
    // Datapoint Count Characteristic.
    {
      AMPEROMETRY_SERVICE_DATAPOINT_COUNT_CHAR_ID,
      (uint8_t *)&AmperometryServiceDatapointCountCharData,
      sizeof(AmperometryServiceDatapointCountCharData),
      (PROPERTY_READABLE | PROPERTY_WRITABLE)
    },
    // Potential Characteristic.
    {
      AMPEROMETRY_SERVICE_POTENTIAL_CHAR_ID,
      (uint8_t *)&AmperometryServicePotentialCharData,
      sizeof(AmperometryServicePotentialCharData),
      (PROPERTY_READABLE | PROPERTY_WRITABLE)
    },
    // Equilibrium Period Characteristic.
    {
      AMPEROMETRY_SERVICE_EQUILIBRIUM_PERIOD_CHAR_ID,
      (uint8_t *)&AmperometryServiceEquilibriumPeriodCharData,
      sizeof(AmperometryServiceEquilibriumPeriodCharData),
      (PROPERTY_READABLE | PROPERTY_WRITABLE)
    },
    // Range Characteristic.
    {
      AMPEROMETRY_SERVICE_RANGE_CHAR_ID,
      (uint8_t *)&AmperometryServiceRangeCharData,
      sizeof(AmperometryServiceRangeCharData),
      (PROPERTY_READABLE | PROPERTY_WRITABLE)
    },
    // Datapoint Characteristic.
    {
      AMPEROMETRY_SERVICE_DATAPOINT_CHAR_ID,
      (uint8_t *)&AmperometryServiceDatapointCharData,
      sizeof(AmperometryServiceDatapointCharData),
      (PROPERTY_READABLE)
    },
    
    // Device Control Service.
    // Command Point Characteristic.
    {
      DEV_CTRL_SERVICE_COMMAND_POINT_CHAR_ID,
      (uint8_t *)&DevCtrlServiceCommandPointCharData,
      sizeof(DevCtrlServiceCommandPointCharData),
      (PROPERTY_WRITABLE)
    },
    // Command Response Characteristic.
    {
      DEV_CTRL_SERVICE_COMMAND_RESPONSE_CHAR_ID,
      (uint8_t *)&DevCtrlServiceCommandResponseCharData,
      sizeof(DevCtrlServiceCommandResponseCharData),
      (PROPERTY_READABLE)
    },
    // Status Characteristic.
    {
      DEV_CTRL_SERVICE_STATUS_CHAR_ID,
      (uint8_t *)&DevCtrlServiceStatusCharData,
      sizeof(DevCtrlServiceStatusCharData),
      (PROPERTY_READABLE)
    },
    // Battery Level Characteristic.
    {
      DEV_CTRL_SERVICE_BATTERY_LEVEL_CHAR_ID,
      (uint8_t *)&DevCtrlServiceBatteryLevelCharData,
      sizeof(DevCtrlServiceBatteryLevelCharData),
      (PROPERTY_READABLE)
    }
  };

/* Exported function implementations -----------------------------------------*/
void DeviceManager_Setup(void)
{
  // Check the state compability.
  if (State == DEVICE_MANAGER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Device manager module Setup function called when not uninitialized.");
  }
  
  // Initialize eeprom emulator.
  EepromEmulator_Init();
  
  // Make the setup of the characteristic server.
  CharacteristicServer_SetupParams_t params;
  
  params.numOfChars = sizeof(CharacteristicDatabase) / sizeof(CharacteristicDatabase[0]);
  params.pCharTable = CharacteristicDatabase;
  params.connectionStateChangedDelegate = connectionStateChangedEventHandler;
  params.writeDelegate = writeEventHandler;
  
  CharacteristicServer_Setup(&params);
  
  State = DEVICE_MANAGER_STATE_READY;
}

// TODO: Implementation.
void DeviceManager_Start(void)
{
  // Check the state compability.
  if (State != DEVICE_MANAGER_STATE_READY)
  {
    ExceptionHandler_ThrowException(\
      "Device manager module start function called when not ready.");
  }
  
  // Start characteristic server.
  CharacteristicServer_Start();
  
  State = DEVICE_MANAGER_STATE_OPERATING;
  DeviceStatus = DEVICE_STATUS_IDLE;
}

// TODO: Implementation.
void DeviceManager_Execute(void)
{
  // Check the state compability.
  if (State != DEVICE_MANAGER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Device manager module Execute function called when not operating.");
  }
  
  // Execute submodules.
  CharacteristicServer_Execute();
  
  // Run operating measurement module.
  if (DeviceStatus == DEVICE_STATUS_AMPEROMETRY_MEASUREMENT)
  {
    Amperometry_Execute();
  } 
}

// TODO: Implementation.
void DeviceManager_Stop(void)
{
  // Check the state compability.
  if (State != DEVICE_MANAGER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Device manager server module Stop function called when not operating.");
  }
  
  State = DEVICE_MANAGER_STATE_READY;
}

/* Private function implementations ------------------------------------------*/
static void writeEventHandler(uint16_t charId)
{
  // Take action according to written characteristic.
  switch(charId)
  {
    // If command point is written, set related event.
  case DEV_CTRL_SERVICE_COMMAND_POINT_CHAR_ID:
    {
      CommandResp_t resp;
      
      // Take action according to command.
      switch (DevCtrlServiceCommandPointCharData)
      {
      case COMMAND_START_AMPEROMETRY:
        {
          if (DeviceStatus != DEVICE_STATUS_IDLE)
          {
            resp = COMMAND_RESP_STATE_NOT_COMPATIBLE;
          }
          else if (checkAmperometryParameters())
          {
            startAmperometry();
            DeviceStatus = DEVICE_STATUS_AMPEROMETRY_MEASUREMENT;
            resp = COMMAND_RESP_SUCCESS;
          }
          else
          {
            resp = COMMAND_RESP_INVALID_SETUP_PARAMETER;
          }
        }
        break;
        
      case COMMAND_STOP_MEASUREMENT:
        {
          // If doing amperometry measurement, stop amperometry module.
          if (DeviceStatus == DEVICE_STATUS_AMPEROMETRY_MEASUREMENT)
          {
            Amperometry_Stop();
            // CharacteristicServer_ClearNotifications();
            DeviceStatus = DEVICE_STATUS_IDLE;
          }
          
          resp = COMMAND_RESP_SUCCESS;
        }
        break;
        
      default:
        resp = COMMAND_RESP_INVALID_COMMAND;
        break;
      }
      
      // Update command response characteristic.
      CharacteristicServer_UpdateCharacteristic(DEV_CTRL_SERVICE_COMMAND_RESPONSE_CHAR_ID, 
                                                (uint8_t *)&resp, sizeof(resp));
      
      // Update device status characteristic.
      CharacteristicServer_UpdateCharacteristic(DEV_CTRL_SERVICE_STATUS_CHAR_ID,
                                                (uint8_t *)&DeviceStatus,
                                                sizeof(DeviceStatus));
    }
    break;
  }
}

// TODO: Amperometry module state is assumed as ready or unitialized. This may
// lead some problems. Need to be careful about this.
static void startAmperometry(void)
{
  // Set params.
  Amperometry_SetupParams_t params;
    
  params.datapointCount = AmperometryServiceDatapointCountCharData;
  params.equilibriumPeriod = AmperometryServiceEquilibriumPeriodCharData;
  params.potential = AmperometryServicePotentialCharData;
  params.samplingFrequency = AmperometryServiceSamplingFrequencyCharData;
  params.feedbackPath = getFBPath((Range_t)AmperometryServiceRangeCharData);
  params.maxRelSamplingFreqErr = 0.001;
  params.currentGainCorrection = 1.0;
  params.currentOffsetCorrection = 1.0;
  params.measurementCompletedDelegate = measurementCompletedEventHandler;
  params.newDatapointDelegate = amperometryNewDatapointEventHandler;
    
  Amperometry_Setup(&params);
    
  // Set calibration relay state on.
  Board_SetCalibrationRelayState(BOARD_CALIBRATION_RELAY_STATE_OFF);
  
  // Start amperometry.
  Amperometry_Start();
}
  
static Bool_t checkAmperometryParameters(void)
{
  // Check parameters.
  if (\
    AMPEROMETRY_SERVICE_IS_VALID_SAMPLING_FREQUENCY(AmperometryServiceSamplingFrequencyCharData) && \
    AMPEROMETRY_SERVICE_IS_VALID_DATAPOINT_COUNT(AmperometryServiceDatapointCountCharData) && \
    AMPEROMETRY_SERVICE_IS_VALID_POTENTIAL(AmperometryServicePotentialCharData) && \
    AMPEROMETRY_SERVICE_IS_VALID_RANGE(AmperometryServiceRangeCharData) && \
    AMPEROMETRY_SERVICE_IS_VALID_EQUILIBRIUM_PERIOD(AmperometryServiceEquilibriumPeriodCharData)\
      )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

// TODO: NOTHING.
static Board_TIAFBPath_t getFBPath(Range_t range)
{
  Board_TIAFBPath_t retval;
  
  switch (range)
  {
  case RANGE_1MA:
    retval = BOARD_TIA_FB_PATH_0;
    break;
    
  case RANGE_100UA:
    retval = BOARD_TIA_FB_PATH_1;
    break;
    
  case RANGE_10UA:
    retval = BOARD_TIA_FB_PATH_2;
    break;
    
  case RANGE_1UA:
    retval = BOARD_TIA_FB_PATH_3;
    break;
    
  case RANGE_100NA:
    retval = BOARD_TIA_FB_PATH_4;
    break;
  }
  
  return retval;
}

// TODO: NOTHING.
static void amperometryNewDatapointEventHandler(float datapoint)
{
  // Update characteristic. This will send notification to the client.
  CharacteristicServer_UpdateCharacteristic(AMPEROMETRY_SERVICE_DATAPOINT_CHAR_ID, 
                                            (uint8_t *)&datapoint, 
                                            sizeof(datapoint));
}

// TODO: Implementation. Handles both amperometry and eis measurement completed events.
static void measurementCompletedEventHandler(void)
{
  // Update device status.
  DeviceStatus = DEVICE_STATUS_IDLE;
  
  // Update related characteristic.
  CharacteristicServer_UpdateCharacteristic(DEV_CTRL_SERVICE_STATUS_CHAR_ID,
                                            (uint8_t *)&DeviceStatus,
                                            sizeof(DeviceStatus));
}

// TODO: Timeout occurred. This is a problem. Stop the device(if it's operating).
// Stop every operation. 
static void connectionStateChangedEventHandler(Bool_t isConnected)
{
  if (isConnected)
  {

  }
  else
  {
    // If doing amperometry measurement, stop amperometry module.
    if (DeviceStatus == DEVICE_STATUS_AMPEROMETRY_MEASUREMENT)
    {
      Amperometry_Stop();
      DeviceStatus = DEVICE_STATUS_IDLE;
    }
    
    // Update characteristic(it won't be notified since the device is disconnected).
    CharacteristicServer_UpdateCharacteristic(DEV_CTRL_SERVICE_STATUS_CHAR_ID,
                                              (uint8_t *)&DeviceStatus,
                                              sizeof(DeviceStatus));
  }
}