#ifndef __CHARACTERISTIC_SERVER_H
#define __CHARACTERISTIC_SERVER_H

#include "generic.h"

/* Exported constants --------------------------------------------------------*/
// Properties
#define CHARACTERISTIC_SERVER_CHAR_PROP_READABLE                0x01
#define CHARACTERISTIC_SERVER_CHAR_PROP_WRITABLE                0x02
#define CHARACTERISTIC_SERVER_CHAR_PROP_VARIABLE_LENGTH         0x04
#define CHARACTERISTIC_SERVER_CHAR_PROP_REGISTERED              0x08              

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  CHARACTERISTIC_SERVER_STATE_UNINIT = 0x00,
  CHARACTERISTIC_SERVER_STATE_READY,
  CHARACTERISTIC_SERVER_STATE_OPERATING
} CharacteristicServer_State_t;

// Delegates.
typedef void (*CharacteristicServer_WriteDelegate_t)(uint16_t charId);
typedef void (*CharacteristicServer_ConnectionStateChangedDelegate_t)(Bool_t isConnected);

// Characteristic struct.
typedef struct
{
  uint16_t charId;
  uint8_t *pData;
  uint32_t length;
  uint8_t properties;
  CharacteristicServer_WriteDelegate_t          writeDelegate;                  // Delegate which is called when write request had been received.
} CharacteristicServer_Characteristic_t;

// Setup parameters.
typedef struct
{
  CharacteristicServer_Characteristic_t         *pCharTable;            // Table which holds characteristics.
  uint16_t                                      numOfChars;             // Number of characteristics.
  CharacteristicServer_ConnectionStateChangedDelegate_t         connectionStateChangedDelegate; // Delegate, which is triggered either connected or disconnected.
  CharacteristicServer_WriteDelegate_t          writeDelegate;          // Delegate, which is triggered when a characteristic is written.
} CharacteristicServer_SetupParams_t;

/* Exported functions --------------------------------------------------------*/
extern void CharacteristicServer_Setup(CharacteristicServer_SetupParams_t *pSetupParams);
extern void CharacteristicServer_Start(void);
extern void CharacteristicServer_Execute(void);
extern void CharacteristicServer_Stop(void);
extern void CharacteristicServer_ClearNotifications(void);
extern void CharacteristicServer_UpdateCharacteristic(uint16_t charId, uint8_t *pData, 
                                                      uint32_t dataLength);
#endif