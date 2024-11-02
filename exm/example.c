/*!
@file   example.c
@brief  Example program for flash memory operations
@t.odo  -
---------------------------------------------------------------------------
*/

#include "lib_stm32flash_emu.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/******************************************************************************
* Helper Functions
******************************************************************************/

void random_delay(unsigned int delay_type)
{
    int delay = (rand() % 5) + 1;
    if (delay_type == 0) {
        printf("Waiting for %d seconds...\n", delay);
        sleep(delay);
    } else {
        printf("Waiting for %d milliseconds...\n", delay);
        usleep(delay * 100000);
    }
}

uint64_t rnd64(uint64_t n)
{
    const uint64_t z = 0x9FB21C651E98DF25;
    n ^= ((n << 49) | (n >> 15)) ^ ((n << 24) | (n >> 40));
    n *= z;
    n ^= n >> 35;
    n *= z;
    n ^= n >> 28;
    return n;
}

/******************************************************************************
* Main Program
******************************************************************************/

int main()
{
    srand(time(NULL));

    uint32_t address = 0x00000010;
    uint64_t data = rand();

    printf("Attempting to program before unlocking the flash...\n");
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data) != HAL_OK) {
        printf("Failed to program data: Flash is locked.\n");
    }
    random_delay(0);

    if (HAL_FLASH_Unlock() != HAL_OK) {
        printf("Failed to unlock flash.\n");
        return -1;
    }
    printf("Flash unlocked successfully.\n");
    random_delay(0);

    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = 0;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t sectorError = 0;

    printf("Erasing sector %d.\n", eraseInit.Sector);
    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK) {
        printf("Failed to erase sector %d.\n", eraseInit.Sector);
        HAL_FLASH_Lock();
        return -1;
    }
    printf("Sector %d erased successfully.\n", eraseInit.Sector);
    random_delay(0);

    for (uint32_t i = 0; i <= 0x100; i += 8) {
        uint32_t prog_address = address + i;
        uint64_t random_data = rnd64(i);
        printf("Programming data at aligned address 0x%08X with data 0x%016lX...\n", prog_address, random_data);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, prog_address, random_data) != HAL_OK) {
            printf("Failed to program data.\n");
        } else {
            printf("Data programmed successfully at address 0x%08X.\n", prog_address);
        }
        random_delay(1);
    }

    if (HAL_FLASH_Lock() != HAL_OK) {
        printf("Failed to lock flash.\n");
        return -1;
    }
    printf("Flash locked successfully.\n");

    return 0;
}
