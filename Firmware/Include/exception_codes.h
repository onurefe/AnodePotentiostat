/**
  * @author     Onur Efe
  */

#ifndef __EXCEPTION_CODES_H
#define __EXCEPTION_CODES_H

typedef enum
{
  NO_EXCEPTION_OCCURED          = 0x00,
  MODULE_NOT_INITIALIZED        = 0x01,
  INCOMPATIBLE_STATE            = 0x02,
  INVALID_PARAMETER             = 0x03,
  UNABLE_TO_PERFORM_OPERATION   = 0x04,
  BUFFER_OVERFLOW               = 0x05,
  FLASH_WRITE_ERROR             = 0x06,
  FLASH_ERASE_ERROR             = 0x07,
  BT_MODULE_ERROR               = 0x08
} ExceptionCode_t;

#endif