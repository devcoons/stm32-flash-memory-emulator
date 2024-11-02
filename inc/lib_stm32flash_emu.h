/*!
@file   lib_stm32flash_emu.h
@brief  Header file for STM32 Flash Emulation Library
@t.odo  -
---------------------------------------------------------------------------

MIT License
Copyright Io.D (devcoons) (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/

#ifndef LIB_STM32FLASH_EMU_H
#define LIB_STM32FLASH_EMU_H

#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Enumerations, structures & Variables
******************************************************************************/

/* --- HAL Status Type Definition ------------------------------------------ */

typedef enum
{
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

/* --- Flash Erase Initialization Structure Definition --------------------- */

typedef struct
{
    uint32_t TypeErase;    /* Type of erase operation (sector or mass erase) */
    uint32_t Banks;        /* Selects the bank to be erased (if applicable) */
    uint32_t Sector;       /* Initial sector to erase when performing a sector erase */
    uint32_t NbSectors;    /* Number of sectors to be erased */
    uint32_t VoltageRange; /* Device voltage range which defines the erase parallelism */
} FLASH_EraseInitTypeDef;

/******************************************************************************
 * Flash Program Type Definitions
******************************************************************************/

#define FLASH_TYPEPROGRAM_BYTE        0x00U
#define FLASH_TYPEPROGRAM_HALFWORD    0x01U
#define FLASH_TYPEPROGRAM_WORD        0x02U
#define FLASH_TYPEPROGRAM_DOUBLEWORD  0x03U

/* --- Flash Voltage Range Definitions ------------------------------------- */

#define FLASH_VOLTAGE_RANGE_1         0x00U 
#define FLASH_VOLTAGE_RANGE_2         0x01U 
#define FLASH_VOLTAGE_RANGE_3         0x02U 
#define FLASH_VOLTAGE_RANGE_4         0x03U 

#define FLASH_TYPEERASE_SECTORS       0x00U  
#define FLASH_TYPEERASE_MASSERASE     0x01U  

/******************************************************************************
 * Flash Error Codes
******************************************************************************/

#define HAL_FLASH_ERROR_NONE          0x00U
#define HAL_FLASH_ERROR_PROG          0x01U
#define HAL_FLASH_ERROR_WRP           0x02U
#define HAL_FLASH_ERROR_PGA           0x04U
#define HAL_FLASH_ERROR_SIZ           0x08U
#define HAL_FLASH_ERROR_PGS           0x10U
#define HAL_FLASH_ERROR_MIS           0x20U
#define HAL_FLASH_ERROR_FAST          0x40U
#define HAL_FLASH_ERROR_RD            0x80U
#define HAL_FLASH_ERROR_OPTV          0x100U

/******************************************************************************
 * Declaration | Public Functions
******************************************************************************/

HAL_StatusTypeDef HAL_FLASH_Unlock(void);
HAL_StatusTypeDef HAL_FLASH_Lock(void);
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);
HAL_StatusTypeDef HAL_FLASHEx_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit);
HAL_StatusTypeDef HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint64_t Data);
uint32_t HAL_FLASH_GetError(void);

/******************************************************************************
 * Callback Function Prototypes (Weak)
******************************************************************************/

void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue) __attribute__((weak));
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue) __attribute__((weak));

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
