/**
  * @author     Onur Efe
  */


#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdint.h>
#include "stm32f4xx.h"

#define SYS_TICK_FREQ           ((uint32_t)1000)
#define M_PI                    ((float)3.14159265358)
#define M_ROOT_OF_2             ((float)1.41421356237)
#define MAX_UINT12              ((uint16_t)4095)
#define MAX_UINT16              ((uint16_t)65535)
#define MAX_INT16               ((int16_t)32767)
#define MAX_UINT32              ((uint32_t)4294967295)
#define NULL                    ((void *)0)
#define FALSE                   (0)
#define TRUE                    (1)

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  OPERATION_RESULT_FAILURE      = 0x00,
  OPERATION_RESULT_SUCCESS      = 0x01
} OperationResult_t;

typedef uint8_t Bool_t;

#endif