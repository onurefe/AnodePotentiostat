/**
  * @author     Onur Efe
  */

/* Includes ------------------------------------------------------------------*/
#include "exception_handler.h"

/* Private constants ---------------------------------------------------------*/
#define EXCEPTION_MESSAGE_MAX_LENGTH    200

/* Private variables ---------------------------------------------------------*/
volatile uint8_t ExceptionMessage[EXCEPTION_MESSAGE_MAX_LENGTH];

/* Exported functions --------------------------------------------------------*/

/***
  * @Brief      Throws an exception message and locks the system.
  *
  * @Param      pExceptionMessage: Pointer to the exception message string. 
  *             In other words null terminated char array.
  * @Return     None.
  */
void ExceptionHandler_ThrowException(const uint8_t *pExceptionMessage)
{
  // Copy message.
  for (uint8_t i = 0; i < EXCEPTION_MESSAGE_MAX_LENGTH; i++)
  {
    ExceptionMessage[i] = pExceptionMessage[i];
    
    if (ExceptionMessage[i] == '\n')                    // Break the loop if null character is found.
    {
      break;
    }
  }
  
  // Lock the system.
  while(TRUE);
}