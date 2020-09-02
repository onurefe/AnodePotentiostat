#ifndef __DEVICE_MANAGER_H
#define __DEVICE_MANAGER_H

/* Include files -------------------------------------------------------------*/
#include "generic.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  DEVICE_MANAGER_STATE_UNINIT = 0x00,
  DEVICE_MANAGER_STATE_READY,
  DEVICE_MANAGER_STATE_OPERATING
} DeviceManager_State_t;

/* Exported functions --------------------------------------------------------*/
extern void DeviceManager_Setup(void);
extern void DeviceManager_Start(void);
extern void DeviceManager_Execute(void);
extern void DeviceManager_Stop(void);

#endif