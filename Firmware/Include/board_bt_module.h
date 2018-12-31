#ifndef __BOARD_BT_MODULE_H
#define __BOARD_BT_MODULE_H

#include "generic.h"

extern void Board_BTModule_Open(void);
extern void Board_BTModule_Close(void);
extern void Board_BTModule_Reset(void);
extern uint8_t Board_BTModule_IsDataPresent(void);
extern int32_t Board_BTModule_ReadAll(uint8_t *buffer, uint8_t buff_size);
extern void Board_BTModule_WriteSerial(const void* data1, const void* data2, 
                                       int32_t n_bytes1, int32_t n_bytes2);

#endif