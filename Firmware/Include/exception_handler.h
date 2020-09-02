/**
  * @author     Onur Efe
  */

#ifndef __EXCEPTION_HANDLER_H
#define __EXCEPTION_HANDLER_H

/* Includes ------------------------------------------------------------------*/
    
#include "generic.h"
#include "exception_codes.h"


/* Exported functions --------------------------------------------------------*/
extern void ExceptionHandler_ThrowException(const uint8_t *pExceptionMessage);

#endif