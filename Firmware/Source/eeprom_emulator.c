/**
  ******************************************************************************
  * @file    eeprom_emulator.c
  * @author  Onur Efe
  * @date    18.03.2017
  * @brief   EEPROM Emulator class implementation.
  ******************************************************************************
  *
  * Copyright (c) 2017 VIVOSENS BIOTECHNOLOGY
  *
  * This program is free software; you can redistribute it and/or modify it 
  * under the terms of the GNU General Public License as published by the Free 
  * Software Foundation; either version 2 of the License, or (at your option) 
  * any later version. This program is distributed in the hope that it will be 
  * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
  * Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
  *
  ******************************************************************************
  */
    
/* Includes ------------------------------------------------------------------*/

#include "stm32f4xx_conf.h"
#include "exception_handler.h"
#include "eeprom_emulator.h"

    
/* Private types -------------------------------------------------------------*/
/* Entry_t struct typedef. Size of the struct should be four bytes */
typedef enum
{
  UNINITIALIZED         = 0x00,
  INITIALIZED           = 0x01 
} State_t;

typedef struct
{
  uint16_t              id;
  uint16_t              length;
  uint32_t              data_offset;
} Entry_t;

typedef struct
{
  const uint32_t        base_address;
  const uint32_t        entry_stack_address;
  const uint32_t        data_stack_address;
  const uint32_t        flash_sector;
  const uint8_t         page_id;
  uint16_t              header;
  uint32_t              entry_stack_pointer;
  uint32_t              data_stack_pointer;
} Page_t;

/* Private constants ---------------------------------------------------------*/

#if !defined(FLASH_VOLTAGE_RANGE_1) && !defined(FLASH_VOLTAGE_RANGE_2) && \
    !defined(FLASH_VOLTAGE_RANGE_3) && !defined(FLASH_VOLTAGE_RANGE_4)
      
#define FLASH_VOLTAGE_RANGE_3

#endif
      
#define PAGE_SIZE                               0x00020000                      /* Page size = 128KByte */
#define PAGE_HEADER_SIZE                        0x02
#define ENTRY_STACK_SIZE                        (0x00004000 - 0x00000008)       /* Entry stack size = 16Kbyte-8Bytes */
#define DATA_STACK_SIZE                         (PAGE_SIZE - (PAGE_HEADER_SIZE + ENTRY_STACK_SIZE))

#define PAGE_A_ID                               0x00
#define PAGE_A_BASE_ADDRESS                     ((uint32_t)(0x080C0000))
#define PAGE_A_SECTOR                           FLASH_Sector_10

#define PAGE_B_ID                               0x01
#define PAGE_B_BASE_ADDRESS                     ((uint32_t)(0x080E0000))
#define PAGE_B_SECTOR                           FLASH_Sector_11

#define PAGE_A_ENTRY_STACK_BASE_ADDRESS         PAGE_A_BASE_ADDRESS + 0x00000008
#define PAGE_A_DATA_STACK_BASE_ADDRESS          (uint32_t)(PAGE_A_ENTRY_STACK_BASE_ADDRESS + ENTRY_STACK_SIZE)

#define PAGE_B_ENTRY_STACK_BASE_ADDRESS         PAGE_B_BASE_ADDRESS + 0x00000008
#define PAGE_B_DATA_STACK_BASE_ADDRESS          (uint32_t)(PAGE_B_ENTRY_STACK_BASE_ADDRESS + ENTRY_STACK_SIZE)

#define PAGE_EMPTY                              ((uint16_t)0xFFFF)
#define PAGE_RECEIVING                          ((uint16_t)0xEEEE)
#define PAGE_ACTIVE                             ((uint16_t)0xAAAA)

#define NULL_OBJECT_ID                          ((uint16_t)0xFFFF)

/* Private functions ---------------------------------------------------------*/
static void                     toggleActivePage(void);
static void                     initPage(Page_t *pPage);
static void                     formatPage(Page_t *pPage, uint16_t page_header);
static void                     flashErase(uint32_t sector);
static void                     flashRead(uint32_t addr, uint8_t *pData, uint16_t length);
static void                     flashWrite(uint32_t addr, uint8_t *pData, uint16_t length);


/* Private variables ---------------------------------------------------------*/
Page_t PageA = 
{
  .base_address = PAGE_A_BASE_ADDRESS, 
  .entry_stack_address = PAGE_A_ENTRY_STACK_BASE_ADDRESS,
  .data_stack_address = PAGE_A_DATA_STACK_BASE_ADDRESS,
  .flash_sector = PAGE_A_SECTOR,
  .page_id = PAGE_A_ID,
  .header = PAGE_EMPTY,
  .entry_stack_pointer = 0,
  .data_stack_pointer = 0
};

Page_t PageB = 
{
  .base_address = PAGE_B_BASE_ADDRESS, 
  .entry_stack_address = PAGE_B_ENTRY_STACK_BASE_ADDRESS,
  .data_stack_address = PAGE_B_DATA_STACK_BASE_ADDRESS,
  .flash_sector = PAGE_B_SECTOR,
  .page_id = PAGE_B_ID,
  .header = PAGE_EMPTY,
  .entry_stack_pointer = 0,
  .data_stack_pointer = 0
};

Page_t  *pActivePage;
State_t State = UNINITIALIZED;
uint8_t TransferBuffer[0x20];

/**
  * @brief  Eeprom emulator module initializer. Should be called before any other 
  *         functions at this module could be used.
  *
  * @param  None.
  *
  * @retval None.
  */
void EepromEmulator_Init(void)
{
  /* Init pages. */
  initPage(&PageA);
  initPage(&PageB);
  
  /* Set active page pointer and format pages */
  if (PageA.header == PAGE_ACTIVE)
  {
    /* PageB header isn't empty, format page B*/
    if (PageB.header != PAGE_EMPTY)
    {
      formatPage(&PageB, PAGE_EMPTY);
    }
    
    pActivePage = &PageA;
  }
  else if (PageB.header == PAGE_ACTIVE)
  {
    /* PageA header isn't empty, format page Az*/
    if (PageA.header != PAGE_EMPTY)
    {
      formatPage(&PageA, PAGE_EMPTY);
    }
    
    pActivePage = &PageB;
  }
  else
  {
    /* If neither of the pages is active, format both of them and select page A as active page */
    formatPage(&PageA, PAGE_ACTIVE);
    formatPage(&PageB, PAGE_EMPTY);
    
    pActivePage = &PageA;
  }
  
  State = INITIALIZED;
}

/**
  * @brief  Reads data object from the flash.
  *
  * @param  objectId: Data object id.
  * @param  offset: Offset of the read start address.
  * @param  maxLength: Maximum read length.
  * @param  pLength: Pointer to return length of the data object.
  * @param  pData: Pointer to return data.
  *
  * @retval True of False(If the object has been found, true).
  */
uint8_t EepromEmulator_ReadObject(uint16_t objectId, uint16_t offset, uint16_t maxLength, 
                                 uint16_t *pLength, uint8_t *pData)
{
  Entry_t entry;
  uint32_t addr;
  uint8_t object_found = FALSE;
  
  /* Parameter validation */
  if (objectId == NULL_OBJECT_ID)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator read function called with invalid\
                                     object ID.\n");
  }
  
  /* Check if the module is initialized */
  if (State != INITIALIZED)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator read function called without\
                                     initializing the module.\n");
  }
  
  /* Check if the stack is empty */
  if (pActivePage->entry_stack_pointer == 0)
  {
    return FALSE;
  }
  
  /* Scan entries in a reverse order, break when the entry at given id found */
  uint16_t entry_index = pActivePage->entry_stack_pointer;
  do
  {
    /* Decrease entry scan pointer by one */
    entry_index--;
    
    /* Read entry */
    addr = pActivePage->entry_stack_address + entry_index * sizeof(Entry_t);
    flashRead(addr, ((uint8_t *)&entry), sizeof(Entry_t));
    
    /* If the entry has been found, set object_found flag */
    if (entry.id == objectId)
    {
      object_found = TRUE;
      break;
    }
  } while (entry_index > 0);

  if (object_found != TRUE)
  {
    return FALSE;
  }
  
  uint16_t read_len;
  
  /* Set data length */
  if (entry.length > (maxLength + offset))
  {
    read_len = maxLength;
  }
  else if (entry.length > offset)
  {
    read_len = entry.length - offset;
  }
  else
  {
    read_len = 0;
  }
  
  *pLength = read_len;
  addr = pActivePage->data_stack_address + entry.data_offset + offset;
  flashRead(addr, pData, read_len);

  return TRUE;
}

/**
  * @brief  Writes data object to the flash memory. 
  *
  * @param  objectId: Id of the data object which is to be written.
  * @param  length: Length of the data object.
  * @param  pData: Pointer of the data.
  *
  * @retval None.
  */
void EepromEmulator_WriteObject(uint16_t objectId, uint16_t length, uint8_t *pData)
{
  Entry_t entry;
  uint32_t addr;
  
  /* Parameter validation */
  if (objectId == NULL_OBJECT_ID)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator write function called with invalid\
                                     object ID.\n");
  }
  
  /* Check if the module is initialized */
  if (State != INITIALIZED)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator write function called without\
                                     initializing the module.\n");
  }

  /* Check if entry stack or data stack overflow will occur */
  if (((pActivePage->entry_stack_pointer + 1) > ((ENTRY_STACK_SIZE / sizeof(Entry_t)) - 2)) || \
      ((pActivePage->data_stack_pointer + length) > (DATA_STACK_SIZE - 1)))
  {
    toggleActivePage();
  }
  
  /* Set entry object members */
  entry.id = objectId;
  entry.length = length;
  entry.data_offset = pActivePage->data_stack_pointer;
  
  /* Write entry to the entry stack. */
  addr = pActivePage->entry_stack_address + pActivePage->entry_stack_pointer * sizeof(Entry_t);
  flashWrite(addr, ((uint8_t *)&entry), sizeof(Entry_t));

  /* Write data to the data stack */
  addr = pActivePage->data_stack_address + pActivePage->data_stack_pointer;
  flashWrite(addr, pData, length);

  /* Update stack pointers */
  pActivePage->entry_stack_pointer++;
  pActivePage->data_stack_pointer += length;
}

/**
  * @brief  Toggles active page and moves current active page objects to the new
  *         active page.
  *
  * @param  None.   
  *
  * @retval None.
  */
static void toggleActivePage(void)
{
  Page_t *p_src_page, *p_dest_page;
  
  /* Set destination and source page pointers */
  p_src_page = pActivePage;
  if (pActivePage->page_id == PAGE_A_ID)
  {
    p_dest_page = &PageB;
  }
  else
  {
    p_dest_page = &PageA;
  }
  
  /* Format destination page(RECEIVING) */
  formatPage(p_dest_page, PAGE_EMPTY);

  /* Search for each entry id, move the value to the new page when found. */
  for (uint16_t scan_id = 0; scan_id < NULL_OBJECT_ID; scan_id++)
  {
    Entry_t entry_read, entry_written;
    uint32_t addr;
    uint16_t read_entry_index;
    uint8_t object_found;
    
    read_entry_index = pActivePage->entry_stack_pointer;
    object_found = FALSE;
    
    /* Scan entries in a reverse order, break when the entry at given id found */
    do
    {
      read_entry_index--;
      
      /* Read entry */
      addr = p_src_page->entry_stack_address + read_entry_index * sizeof(Entry_t);
      flashRead(addr, ((uint8_t *)&entry_read), sizeof(Entry_t));
      
      /* If the entry has been found, set object_found flag */
      if (entry_read.id == scan_id)
      {
        object_found = TRUE;
        break;
      }
    } while (read_entry_index > 0);
      
    if (object_found == FALSE)
    {
      continue;
    }

    /* Set entry written. */
    entry_written.id = entry_read.id;
    entry_written.length = entry_read.length;
    entry_written.data_offset = p_dest_page->data_stack_pointer;
      
    /* Write entry to destination page */
    addr = p_dest_page->entry_stack_address + p_dest_page->entry_stack_pointer * sizeof(Entry_t);
    flashWrite(addr, ((uint8_t *)&entry_written), sizeof(Entry_t));
    
      
    uint16_t transfer_length;
    uint16_t chunk_offset = 0U;
      
    /* Move data from source to destination. */
    while (entry_read.length > chunk_offset)
    {
      if ((entry_read.length - chunk_offset) >= sizeof(TransferBuffer))
      {
        transfer_length = sizeof(TransferBuffer);
      }
      else
      {
        transfer_length = (entry_read.length - chunk_offset);
      }
        
      /* Move block data from source to TransferBuffer. */
      addr = p_src_page->data_stack_address + entry_read.data_offset + chunk_offset;
      flashRead(addr, TransferBuffer, sizeof(TransferBuffer));

      /* Move block data from TransferBuffer to destination. */
      addr = p_dest_page->data_stack_address + entry_written.data_offset + chunk_offset;
      flashWrite(addr, TransferBuffer, sizeof(TransferBuffer));
      
      /* Update chunk offset. */
      chunk_offset += transfer_length;
    }
      
    /* Update destination page entry and data stack pointers. */
    p_dest_page->entry_stack_pointer++;
    p_dest_page->data_stack_pointer = p_dest_page->data_stack_address +\
                                      entry_written.data_offset + entry_written.length;
  }
  
  /* Format destination page(ACTIVE) */
  formatPage(p_dest_page, PAGE_ACTIVE);
  pActivePage = p_dest_page;
  
  /* Format source page(EMPTY) */
  formatPage(p_dest_page, PAGE_EMPTY);
}

/**
  * @brief  Initializes page object. 
  *
  * @param  pPage: Pointer to the page object.
  *
  * @retval None. 
  */
static void initPage(Page_t *pPage)
{
  Entry_t       entry;
  uint32_t      addr;
  
  /* Parse page header */
  flashRead(pPage->base_address, ((uint8_t *)&(pPage->header)), sizeof(pPage->header));
    
  /* Init entry stack pointer */
  pPage->entry_stack_pointer = 0;
  
  addr = pPage->entry_stack_address;
    
  /* Start scanning from the stack base address */
  for (uint16_t entry_index = 0; entry_index < (ENTRY_STACK_SIZE / sizeof(Entry_t)); 
       entry_index++)
  {
    /* Parse entry object */
    flashRead(addr, ((uint8_t *)&entry), sizeof(Entry_t));

    /* If object ID is NULL_OBJECT_ID */
    if (entry.id == NULL_OBJECT_ID)
    {
      pPage->entry_stack_pointer = entry_index;
      break;
    }
    
    addr += sizeof(Entry_t);
  }

  /* Set entry object elements */
  if (pPage->entry_stack_pointer > 0)
  {
    /* Parse top entry */
    addr = pPage->entry_stack_address + (pPage->entry_stack_pointer - 1) * sizeof(Entry_t);
    flashRead(addr, ((uint8_t *)&entry), sizeof(Entry_t));
    
    pPage->data_stack_pointer = entry.data_offset + entry.length;
  }
  else
  {
    pPage->data_stack_pointer = 0U;
  }
}
 
/**
  * @brief  Formats given page according to it's page header.  
  *
  * @param  pPage: Pointer to the page object.
  * @param  page_header: Page header.
  *
  * @retval None.
  */
static void formatPage(Page_t *pPage, uint16_t page_header)
{
  switch (page_header)
  {
  case PAGE_EMPTY:
    {
      flashErase(pPage->flash_sector);
    }
    break;
    
  case PAGE_RECEIVING:
  case PAGE_ACTIVE:
  default:
    break;
  }
  
  flashWrite(pPage->base_address, ((uint8_t *)&page_header), sizeof(page_header));

  pPage->entry_stack_pointer = 0;
  pPage->data_stack_pointer = 0;
  pPage->header = page_header;
}

/**
  * @brief  Flash erase. 
  *
  * @param  sector: Flash sector to be erased.
  *
  * @retval None.
  */
static void flashErase(uint32_t sector)
{
  FLASH_Unlock();
      
#if defined(FLASH_VOLTAGE_RANGE_1)
      
  if (FLASH_EraseSector(sector, VoltageRange_1) != FLASH_COMPLETE)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator VoltageRange_1 flash erase \
                                     error occurred.");
  }
#endif
      
#if defined(FLASH_VOLTAGE_RANGE_2)
      
  if (FLASH_EraseSector(sector, VoltageRange_2) != FLASH_COMPLETE)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator VoltageRange_2 flash erase \
                                     error occurred.");
  }
#endif
      
#if defined(FLASH_VOLTAGE_RANGE_3)
     
  if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator VoltageRange_3 flash erase \
                                     error occurred.");
  }
#endif
      
#if defined(FLASH_VOLTAGE_RANGE_4)
      
  if (FLASH_EraseSector(sector, VoltageRange_4) != FLASH_COMPLETE)
  {
    ExceptionHandler_ThrowException("EEPROM Emulator VoltageRange_4 flash erase \
                                     error occurred.");
  }
#endif
      
  FLASH_Lock();
}


/**
  * @brief  Reads data from flash memory. 
  *
  * @param  addr: Read address.
  * @param  pData: Data buffer address.
  * @param  length: Length of the data.
  *
  * @retval None.
  */
static void flashRead(uint32_t addr, uint8_t *pData, uint16_t length)
{
  uint16_t index = 0;
  
  FLASH_Unlock();
  
  while (length > index)
  {
    /* Conditional read size according to number of remaining bytes and alignment. */
    if (((length - index) >= 8) && (((addr + index) & 0x00000111) == 0U))
    {
      *((uint64_t *)&pData[index]) = *((volatile uint64_t *)(addr + index));
      
      index += 8;
      continue;
    }
    
    if (((length - index) >= 4) && (((addr + index) & 0x00000011) == 0U))
    {
      *((uint32_t *)&pData[index]) = *((volatile uint32_t *)(addr + index));
      
      index += 4;
      continue;
    }
    
    if (((length - index) >= 2) && (((addr + index) & 0x00000001) == 0U))
    {
      *((uint16_t *)&pData[index]) = *((volatile uint16_t *)(addr + index));
      
      index += 2;
      continue;
    }

    pData[index] = *((volatile uint8_t *)(addr + index));
      
    index += 1;
  }
  
  FLASH_Lock();
}


/**
  * @brief  Writes data to the flash memory. 
  *
  * @param  addr: Data address.
  * @param  pData: Pointer to data which is to be written.
  * @param  length: Length of the data.
  *
  * @retval None.
  */
static void flashWrite(uint32_t addr, uint8_t *pData, uint16_t length)
{
  /* Write data to the flash memory */
  uint16_t index = 0;
  
  FLASH_Unlock();
  
  while (length > index)
  {
    /* Conditional write size according to number of remaining bytes */
#if defined(FLASH_VOLTAGE_RANGE_4)
    
    if (((length - index) >= 8) && (((addr + index) & 0x00000111) == 0U))
    {
      if (FLASH_ProgramDoubleWord((addr + index), 
                                  ((uint64_t *)&pData[index])) != FLASH_COMPLETE)
      {
        ExceptionHandler_ThrowException("EEPROM Emulator quad word flash write \
                                         error occurred.");
      }
      
      index += 8;
      continue;
    }
#endif

#if defined(FLASH_VOLTAGE_RANGE_4) || defined(FLASH_VOLTAGE_RANGE_3)
    
    if (((length - index) >= 4) && (((addr + index) & 0x00000011) == 0U))
    {
      if (FLASH_ProgramWord((addr + index), 
                            *((uint32_t *)&pData[index])) != FLASH_COMPLETE)
      {
        ExceptionHandler_ThrowException("EEPROM Emulator double word flash write \
                                         error occurred.\n");
      }
      
      index += 4;
      continue;
    }
#endif
    
#if defined(FLASH_VOLTAGE_RANGE_4) || defined(FLASH_VOLTAGE_RANGE_3) || \
    defined(FLASH_VOLTAGE_RANGE_2)
      
    if (((length - index) >= 2) && (((addr + index) & 0x00000001) == 0U))
    {
      if (FLASH_ProgramHalfWord((addr + index), 
                                *((uint16_t *)&pData[index])) != FLASH_COMPLETE)
      {
        ExceptionHandler_ThrowException("EEPROM Emulator word flash write \
                                         error occurred.\n");
      }
      
      index += 2;
      continue;
    }
#endif
    
    if (FLASH_ProgramByte((addr + index), pData[index]) != FLASH_COMPLETE)
    {
      ExceptionHandler_ThrowException("EEPROM Emulator byte flash write \
                                       error occurred.\n");
    }
      
    index += 1;
  }
  
  FLASH_Lock();
}