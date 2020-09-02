#ifndef __EIS_SERVICE_H
#define __EIS_SERVICE_H

/* Includes ------------------------------------------------------------------*/
#include "hal_types.h"
#include "bluenrg_gatt_server.h"
#include "bluenrg_gap.h"
#include "string.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "hci_const.h"
#include "gp_timer.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_aci_const.h"   
#include "hci.h"
#include "hci_le.h"
#include "hal.h"
#include "sm.h"
#include "debug.h"
#include "exception_handler.h"

#include <stdlib.h>

/** @addtogroup X-CUBE-BLE1_Applications
 *  @{
 */

/** @addtogroup SensorDemo
 *  @{
 */
 
/** @addtogroup SENSOR_SERVICE 
 * @{
 */

/** @addtogroup SENSOR_SERVICE_Exported_Defines 
 * @{
 */
/* Exported defines ----------------------------------------------------------*/   
#define IDB04A1 0
#define IDB05A1 1

/* Exported functions. */
extern void EISService_AddDeviceConfigurationService(void);
extern void EISService_AddDeviceControlService(void);
extern void EISService_AddMeasurementDataService(void);
#endif