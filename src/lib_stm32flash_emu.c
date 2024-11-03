/*!
@file   lib_stm32flash_emu.c
@brief  Source file for STM32 Flash Emulation Library
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

#include "lib_stm32flash_emu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define FLASH_SIZE       (1 * 1024 * 1024)  /* 1MB Flash size */
#define FLASH_FILENAME   "flash_memory.bin"
#define LOG_FILENAME     "flash_operations.log"

/******************************************************************************
* Static Variables
******************************************************************************/

static uint8_t flash_memory[FLASH_SIZE];
static uint32_t flash_error = HAL_FLASH_ERROR_NONE;
static int flash_locked = 1;
static int flash_busy = 0;
static int flash_initialized = 0;

static pthread_mutex_t flash_mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************
* Static Functions
******************************************************************************/

/* Save the current state of flash memory to a file */
static void save_flash_memory()
{
    FILE *flash_file = fopen(FLASH_FILENAME, "wb");
    if (flash_file) {
        fwrite(flash_memory, sizeof(uint8_t), FLASH_SIZE, flash_file);
        fflush(flash_file);
        fclose(flash_file);
        flash_initialized = 1;
    } else {
        perror("Error opening flash memory file");
    }
}

/* Log flash operation details to a file */
static void log_operation(const char *operation, uint32_t address, uint64_t data, uint32_t data_size, const char *status)
{
    FILE *log_file = fopen(LOG_FILENAME, "a");
    if (log_file)
    {
        time_t now = time(NULL);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log_file, "{\"timestamp\": \"%s\", \"operation\": \"%s\", \"address\": \"0x%08X\", \"data\": \"0x%016lX\", \"size\": \"%d\", \"status\": \"%s\"}\n",
                timestamp, operation, address, data, data_size, status);
        fclose(log_file);
    }
}

/* Check if an address is aligned */
static int is_address_aligned(uint32_t address, uint32_t alignment)
{
    return (address % alignment) == 0;
}

/* Simulate a delay for flash operations */
static void simulate_flash_delay(unsigned int microseconds)
{
    usleep(microseconds);
}

/******************************************************************************
* Public Functions
******************************************************************************/

HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
    pthread_mutex_lock(&flash_mutex);
    if (flash_locked)
    {
        flash_locked = 0;
        log_operation("Unlock", 0, 0, 0, "Success");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_OK;
    }
    else
    {
        flash_error = HAL_FLASH_ERROR_NONE;
        log_operation("Unlock", 0, 0, 0, "Already Unlocked");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
    pthread_mutex_lock(&flash_mutex);
    if (!flash_locked)
    {
        flash_locked = 1;
        log_operation("Lock", 0, 0, 0, "Success");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_OK;
    }
    else
    {
        flash_error = HAL_FLASH_ERROR_NONE;
        log_operation("Lock", 0, 0, 0, "Already Locked");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
    pthread_mutex_lock(&flash_mutex);
    if (flash_locked)
    {
        flash_error = HAL_FLASH_ERROR_WRP;
        log_operation("Program", Address, Data, 0, "Error: Flash Locked");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_ERROR;
    }

    uint32_t alignment;
    switch (TypeProgram)
    {
        case FLASH_TYPEPROGRAM_BYTE:       alignment = 1; break;
        case FLASH_TYPEPROGRAM_HALFWORD:   alignment = 2; break;
        case FLASH_TYPEPROGRAM_WORD:       alignment = 4; break;
        case FLASH_TYPEPROGRAM_DOUBLEWORD: alignment = 8; break;
        default:
            flash_error = HAL_FLASH_ERROR_PROG;
            log_operation("Program", Address, Data, 0, "Error: Invalid TypeProgram");
            pthread_mutex_unlock(&flash_mutex);
            return HAL_ERROR;
    }

    if (!is_address_aligned(Address, alignment))
    {
        flash_error = HAL_FLASH_ERROR_PGA;
        log_operation("Program", Address, Data, alignment, "Error: Address Misaligned");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_ERROR;
    }

    flash_busy = 1;
    simulate_flash_delay(100);

    int error_flag = 0;
    if (TypeProgram == FLASH_TYPEPROGRAM_BYTE)
    {
        if ((flash_memory[Address] & (uint8_t)Data) != (uint8_t)Data) error_flag = 1;
        else flash_memory[Address] &= (uint8_t)Data;
    }
    else if (TypeProgram == FLASH_TYPEPROGRAM_HALFWORD)
    {
        uint16_t *halfword_address = (uint16_t*)&flash_memory[Address];
        if ((*halfword_address & (uint16_t)Data) != (uint16_t)Data) error_flag = 1;
        else *halfword_address &= (uint16_t)Data;
    }
    else if (TypeProgram == FLASH_TYPEPROGRAM_WORD)
    {
        uint32_t *word_address = (uint32_t*)&flash_memory[Address];
        if ((*word_address & (uint32_t)Data) != (uint32_t)Data) error_flag = 1;
        else *word_address &= (uint32_t)Data;
    }
    else if (TypeProgram == FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
        uint64_t *doubleword_address = (uint64_t*)&flash_memory[Address];
        if ((*doubleword_address & Data) != Data) error_flag = 1;
        else *doubleword_address &= Data;
    }

    if (error_flag)
    {
        flash_error = HAL_FLASH_ERROR_PROG;
        log_operation("Program", Address, Data, alignment, "Error: Cannot turn 0s to 1s");
        flash_busy = 0;
        pthread_mutex_unlock(&flash_mutex);
        return HAL_ERROR;
    }

    save_flash_memory();
    flash_busy = 0;

    log_operation("Program", Address, Data, alignment, "Success");
    HAL_FLASH_EndOfOperationCallback(Address);
    pthread_mutex_unlock(&flash_mutex);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError)
{
    pthread_mutex_lock(&flash_mutex);
    if (flash_locked)
    {
        flash_error = HAL_FLASH_ERROR_WRP;
        log_operation("Erase", 0, 0, 0, "Error: Flash Locked");
        pthread_mutex_unlock(&flash_mutex);
        return HAL_ERROR;
    }

    memset(flash_memory, 0xFF, FLASH_SIZE);
    simulate_flash_delay(500);

    if (!flash_initialized) save_flash_memory();
    log_operation("Erase", 0, 0, 0, "Success");

    HAL_FLASH_EndOfOperationCallback(0);
    pthread_mutex_unlock(&flash_mutex);
    return HAL_OK;
}

uint32_t HAL_FLASH_GetError(void)
{
    return flash_error;
}

__attribute__((weak)) void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue) { }
__attribute__((weak)) void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue) { }

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
