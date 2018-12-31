/**
  * @author     Onur Efe
  */

#include "eis_service.h"

#define DEFAULT_FREQUENCY_ARRAY_SIZE            43

/* Device Configuration Service UUID's */
// 583afe95-b7ec-483c-ad69-e3bce26e269c
#define DEV_CFG_SERVICE_UUID                    {0x58, 0x3a, 0xfe, 0x95, 0xb7, 0xec, 0x48, 0x3c, \
                                                 0xad, 0x69, 0xe3, 0xbc, 0xe2, 0x6e, 0x26, 0x9c}
// c4f93c10-2018-4961-8e16-1ae8d378c3a6
#define DEV_CFG_SIGNAL_AC_AMPLITUDE_CHAR_UUID   {0xc4, 0xf9, 0x3c, 0x10, 0x20, 0x18, 0x49, 0x61, \
                                                 0x8e, 0x16, 0x1a, 0xe8, 0xd3, 0x78, 0xc3, 0xa6}
// e5020a91-8a96-4b07-9725-bad9b4e90756
#define DEV_CFG_BIAS_DC_POTENTIAL_CHAR_UUID     {0xe5, 0x02, 0x0a, 0x91, 0x8a, 0x96, 0x4b, 0x07, \
                                                 0x97, 0x25, 0xba, 0xd9, 0xb4, 0xe9, 0x07, 0x56}
// 5e2f6c08-5b1a-4a55-b1ed-3ac204b2b68d
#define DEV_CFG_FREQ_ARRAY_CHAR_UUID            {0x5e, 0x2f, 0x6c, 0x08, 0x5b, 0x1a, 0x4a, 0x55, \
                                                 0xb1, 0xed, 0x3a, 0xc2, 0x04, 0xb2, 0xb6, 0x8d}
// ae015f88-cd02-4d42-affa-fd80429cbdd7
#define DEV_CFG_DP_COUNT_CHAR_UUID              {0xae, 0x01, 0x5f, 0x88, 0xcd, 0x02, 0x4d, 0x42, \
                                                 0xaf, 0xfa, 0xfd, 0x80, 0x42, 0x9c, 0xbd, 0xd7}
// ac6b8fa4-013a-4262-a51c-6fcc99df4484
#define DEV_CFG_EQUILIBRIUM_PERIOD_CHAR_UUID    {0xac, 0x6b, 0x8f, 0xa4, 0x01, 0x3a, 0x42, 0x62, \
                                                 0xa5, 0x1c, 0x6f, 0xcc, 0x99, 0xdf, 0x44, 0x84}
// 55a5444d-7852-4ca9-a72f-1326b480655a
#define DEV_CFG_PRETREATMENT_PERIOD_CHAR_UUID   {0x55, 0xa5, 0x44, 0x4d, 0x78, 0x52, 0x4c, 0xa9, \
                                                 0xa7, 0x2f, 0x13, 0x26, 0xb4, 0x80, 0x65, 0x5a}

/* Device Control Service UUID's */
// 191a0188-5ad6-4b51-b019-d2ddbb19a008
#define DEV_CTRL_SERVICE_UUID                   {0x19, 0x1a, 0x01, 0x88, 0x5a, 0xd6, 0x4b, 0x51, \
                                                 0xb0, 0x19, 0xd2, 0xdd, 0xbb, 0x19, 0xa0, 0x08}
// 2cefcac9-b175-4d22-97ca-a9a3d364bddd
#define DEV_CTRL_COMMAND_POINT_CHAR_UUID        {0x2c, 0xef, 0xca, 0xc9, 0xb1, 0x75, 0x4d, 0x22, \
                                                 0x97, 0xca, 0xa9, 0xa3, 0xd3, 0x64, 0xbd, 0xdd}
// 925d49b0-cdb7-402c-be78-39d8bf884881
#define DEV_CTRL_DEVICE_STATUS_CHAR_UUID        {0x92, 0x5d, 0x49, 0xb0, 0xcd, 0xb7, 0x40, 0x2c, \
                                                 0xbe, 0x78, 0x39, 0xd8, 0xbf, 0x88, 0x48, 0x81}

/* Measurement Data Service UUID's */
// efaf183e-e240-42e9-84f6-3abf0f658e26
#define MEAS_DATA_SERVICE_UUID                  {0xef, 0xaf, 0x18, 0x3e, 0xe2, 0x40, 0x42, 0xe9, \
                                                 0x84, 0xf6, 0x3a, 0xbf, 0x0f, 0x65, 0x8e, 0x26}
// 061a1207-9383-48ea-9dc4-58bc47fe5e5c
#define MEAS_DATA_DP_INDEX_CHAR_UUID            {0x06, 0x1a, 0x12, 0x07, 0x93, 0x83, 0x48, 0xea, \
                                                 0x9d, 0xc4, 0x58, 0xbc, 0x47, 0xfe, 0x5e, 0x5c}
// 4be8e8b0-6ff8-4d3f-8084-0ec4ccc788b2
#define MEAS_DATA_DP_VALUE_CHAR_UUID            {0x4b, 0xe8, 0xe8, 0xb0, 0x6f, 0xf8, 0x4d, 0x3f, \
                                                 0x80, 0x84, 0x0e, 0xc4, 0xcc, 0xc7, 0x88, 0xb2}

/* Private variables ---------------------------------------------------------*/

/* Device configuration service variables. */
uint16_t DevCfgServiceHandle;
uint16_t DevCfgSignalACAmplitudeCharHandle;
uint16_t DevCfgBiasDCPotentialCharHandle;
uint16_t DevCfgFreqArrayCharHandle;
uint16_t DevCfgDPCountCharHandle;
uint16_t DevCfgEquilibriumPeriodCharHandle;
uint16_t DevCfgPreatreatmentPeriodCharHandle;

/* Device control service variables. */
uint16_t DevCtrlServiceHandle;
uint16_t DevCtrlCommandPointCharHandle;
uint16_t DevCtrlDeviceStatusCharHandle;

/* Measurement data service variables. */
uint16_t MeasDataServiceHandle;
uint16_t MeasDataDPIndexCharHandle;
uint16_t MeasDataDPValueCharHandle;


void EISService_AddDeviceConfigurationService(void)
{
  tBleStatus ret;
  
  /* Add device configuration service. */
  {
    uint8_t uuid[16] = DEV_CFG_SERVICE_UUID;
    
    ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 29,
                            &DevCfgServiceHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       service adding error.\n");
    }
  }
  
  /* Add signal AC amplitude characteristic. */
  {
    uint8_t uuid[16] = DEV_CFG_SIGNAL_AC_AMPLITUDE_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCfgServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(float), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCfgSignalACAmplitudeCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       signal AC amplitude characteristic adding error.\n");
    }
  }
  
  /* Add bias DC potential characteristic. */
  {
    uint8_t uuid[16] = DEV_CFG_BIAS_DC_POTENTIAL_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCfgServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(float), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCfgSignalACAmplitudeCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       bias DC potential characteristic adding error.\n");
    }
  }
  
  /* Add frequency array characteristic. */
  {
    uint8_t uuid[16] = DEV_CFG_FREQ_ARRAY_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCfgServiceHandle,
			    UUID_TYPE_128, uuid, 
                            (sizeof(float) * DEFAULT_FREQUENCY_ARRAY_SIZE), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    1,         // Variable length
			    &DevCfgFreqArrayCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       frequency array characteristic adding error.\n");
    }
  }
  
  /* Add datapoint count characteristic. */
  {
    uint8_t uuid[16] = DEV_CFG_DP_COUNT_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCfgServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(uint16_t), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCfgDPCountCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       data point count characteristic adding error.\n");
    }
  }
  
  /* Add equilibrium period characteristic. */
  {
    uint8_t uuid[16] = DEV_CFG_EQUILIBRIUM_PERIOD_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCfgServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(float), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCfgEquilibriumPeriodCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       equilibrium period characteristic adding error.\n");
    }
  }
  
  /* Add pretreatment period characteristic. */
  {
    uint8_t uuid[16] = DEV_CFG_PRETREATMENT_PERIOD_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCfgServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(float), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCfgPreatreatmentPeriodCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device configuration \
                                       pretreatment period characteristic adding error.\n");
    }
  }
}

void EISService_AddDeviceControlService(void)
{
  tBleStatus ret;
  
  /* Add device control service. */
  {
    uint8_t uuid[16] = DEV_CTRL_SERVICE_UUID;
    
    ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 7,
                            &DevCtrlServiceHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device control \
                                       service adding error.\n");
    }
  }
  
  /* Add command point characteristic */
  {
    uint8_t uuid[16] = DEV_CTRL_COMMAND_POINT_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCtrlServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(uint8_t), 
			    CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCtrlCommandPointCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module device control service \
                                       command point characteristic adding error.\n");
    }
  }
  
  /* Add device status characteristic. */
  {
    uint8_t uuid[16] = DEV_CTRL_DEVICE_STATUS_CHAR_UUID;
    
    ret = aci_gatt_add_char(DevCtrlServiceHandle,
                            UUID_TYPE_128, uuid, 
                            sizeof(uint8_t), 
			    CHAR_PROP_READ, 
                            ATTR_PERMISSION_NONE, 
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &DevCtrlDeviceStatusCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {

      ExceptionHandler_ThrowException("EIS Service Bluetooth module device control service \
                                       device status characteristic adding error.\n");
    }
  }
}

void EISService_AddMeasurementDataService(void)
{
  tBleStatus ret;
  
  /* Add measurement data service. */
  {
    uint8_t uuid[16] = MEAS_DATA_SERVICE_UUID;
    
    ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 7,
                            &MeasDataServiceHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module measurement data service \
                                       adding error.\n");
    }
  }
  
  /* Add data point index characteristic */
  {
    uint8_t uuid[16] = MEAS_DATA_DP_INDEX_CHAR_UUID;
    
    ret = aci_gatt_add_char(MeasDataServiceHandle,
			    UUID_TYPE_128, uuid, 
                            sizeof(uint8_t), 
			    CHAR_PROP_READ | CHAR_PROP_WRITE, 
                            ATTR_PERMISSION_NONE,
			    GATT_NOTIFY_ATTRIBUTE_WRITE,
			    16,
			    0,
			    &MeasDataDPIndexCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module measurement data service \
                                       datapoint index characteristic adding error.\n");
    }
  }
  
  /* Add datapoint value characteristic. */
  {
    uint8_t uuid[16] = MEAS_DATA_DP_VALUE_CHAR_UUID;
    
    ret = aci_gatt_add_char(MeasDataServiceHandle,
                            UUID_TYPE_128, uuid, 
                            (sizeof(float) * 2), 
			    CHAR_PROP_READ,
                            ATTR_PERMISSION_NONE,
			    GATT_DONT_NOTIFY_EVENTS,
			    16,
			    0,
			    &MeasDataDPValueCharHandle);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      ExceptionHandler_ThrowException("EIS Service Bluetooth module measurement data service \
                                       datapoint value characteristic adding error.\n");
    }
  }
}