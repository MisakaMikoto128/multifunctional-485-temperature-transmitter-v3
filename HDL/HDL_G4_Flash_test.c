/**
 * @file HDL_G4_Flash_test.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-12
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "HDL_G4_Flash_test.h"
#include "log.h"
#include <string.h>

static uint8_t test_buf[HDL_FLASH_SECTOR_SIZE] = {0};
static uint8_t test_buf2[100] = {0};
static uint8_t test_buf3[100] = {0};
void Flash_inner_fun_test()
{

    HDL_G4_Flash_init();

    uint32_t sector = 29;
    __IO uint32_t addr = HDL_G4_Flash_get_address_by_sector(sector);

    Debug_Printf("test sector : %u , address : %#X\r\n", sector, addr);
    HDL_G4_Flash_erase_one_sector(sector);
    //保证后面一个扇区被擦除，用来看写一个扇区会不会导致另一个扇区数据被擦除。
    HDL_G4_Flash_erase_one_sector(sector + 1);
    for (size_t i = 0; i < HDL_FLASH_SECTOR_SIZE; i++)
    {
        test_buf[i] = i;
    }
    HDL_G4_Flash_write_one_sector_nocheck(sector, test_buf);
    for (size_t i = 0; i < HDL_FLASH_SECTOR_SIZE; i++)
    {
        test_buf[i] = 0;
    }
    HDL_G4_Flash_read(addr, test_buf, HDL_FLASH_SECTOR_SIZE);

    uint32_t error_cnt = 0;
    for (size_t i = 0; i < HDL_FLASH_SECTOR_SIZE; i++)
    {
        if (test_buf[i] != (uint8_t)i)
        {
            error_cnt++;
        }
    }
    Debug_Printf("test sector : %u , error cnt : %u\r\n", sector, error_cnt);
    sector++;
    error_cnt = 0;
    addr = HDL_G4_Flash_get_address_by_sector(sector);
    HDL_G4_Flash_read(addr, test_buf, HDL_FLASH_SECTOR_SIZE);
    for (size_t i = 0; i < HDL_FLASH_SECTOR_SIZE; i++)
    {
        if (test_buf[i] != 0xFF)
        {
            error_cnt++;
        }
    }
    Debug_Printf("test sector : %u , error cnt : %u\r\n", sector, error_cnt);
}

void Flash_test()
{
    /*
    测试写1/4个扇区，写之前先读取扇区剩余部分的数据有没有被修改。
    */
    uint32_t sector = 29;
    uint32_t start_addr = HDL_G4_Flash_get_address_by_sector(sector);

    for (size_t i = 0; i < HDL_FLASH_SECTOR_SIZE; i++)
    {
        test_buf[i] = (uint8_t)(i * i);
    }

    HDL_G4_Flash_read(start_addr, test_buf2, 100);
    HDL_G4_Flash_read(start_addr + 100 + HDL_FLASH_SECTOR_SIZE / 4, test_buf3, 100);
    HDL_G4_Flash_write(start_addr + 100, test_buf, HDL_FLASH_SECTOR_SIZE / 4);
    //清零缓存区域
    memset(test_buf, 0, HDL_FLASH_SECTOR_SIZE);
    HDL_G4_Flash_read(start_addr, test_buf, HDL_FLASH_SECTOR_SIZE);
    uint32_t error_cnt = 0;
    for (size_t i = 0; i < 100; i++)
    {
        if (test_buf[i] != test_buf2[i])
        {
            error_cnt++;
        }
    }

    Debug_Printf("the first 100 bytes,test sector : %u , error cnt : %u\r\n", sector, error_cnt);
    error_cnt = 0;

    for (size_t i = 0; i < 100; i++)
    {
        if (test_buf[i + 100 + HDL_FLASH_SECTOR_SIZE / 4] != test_buf3[i])
        {
            error_cnt++;
        }
    }

    Debug_Printf("the last 100 bytes,test sector : %u , error cnt : %u\r\n", sector, error_cnt);
    error_cnt = 0;
    size_t tmp = 0;
    for (size_t i = 100; i < HDL_FLASH_SECTOR_SIZE / 4 + 100; i++)
    {
				tmp = i - 100;
        if (test_buf[i] != (uint8_t)(tmp * tmp))
        {
            error_cnt++;
        }
    }

    Debug_Printf("the 1/4 sector bytes,test sector : %u , error cnt : %u\r\n", sector, error_cnt);
    error_cnt = 0;
}