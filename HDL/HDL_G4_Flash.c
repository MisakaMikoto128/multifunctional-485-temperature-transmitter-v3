/**
 * @file HDL_G4_Flash.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-12
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "HDL_G4_Flash.h"
#include <string.h>
uint32_t HDL_G4_Flash_get_address_by_sector(uint32_t sector);
int HDL_G4_Flash_write_one_sector_nocheck(uint32_t sector, uint8_t *buf);
int HDL_G4_Flash_erase_one_sector(uint32_t sector);
int HDL_G4_Flash_erase_sector(uint32_t sector, uint32_t count);

static uint8_t flash_buffer[HDL_FLASH_SECTOR_SIZE] = {0};

/**
 * @brief 片内Flash初始化。
 *
 * @return int 0初始化成功，1初始化失败。
 */
int HDL_G4_Flash_init()
{
    return 0;
}

/**
 * @brief 随机写一定数量的数据内部flash。
 *
 * @param address 数据写到内部flash中的地址。
 * @param data 指向待存放数据的指针。
 * @param size 数据长度，单位字节。
 * @return int 成功返回0，失败返回-1。
 */
int HDL_G4_Flash_write(uint32_t address, uint8_t *data, uint32_t size)
{
    int32_t status = 0;
    // 1.计算起始写入的扇区编号和起始写入扇区剩余需要写入的字节数量。
    uint32_t current_sec_waiting_to_write_byte_num = 0; // The number of bytes waiting to be written
    uint32_t current_sec_remain_bytes = 0;              //当前扇区剩余需要写入字节的数量
    uint32_t sec = 0;
    uint32_t current_sec_start_write_addr = 0; //当前扇区写入数据的相对起始地址
    sec = HDL_SECTOR_OF_ADDRESS(address);

    //计算第一个扇区剩余需要写入字节的数量
    current_sec_remain_bytes = HDL_FLASH_SECTOR_SIZE - address % HDL_FLASH_SECTOR_SIZE;
    //如果要写入的字节数量小于等于第一个扇区剩余需要写入字节的数量
    if (size <= current_sec_remain_bytes)
    {
        current_sec_waiting_to_write_byte_num = size;
    }
    else
    {
        current_sec_waiting_to_write_byte_num = current_sec_remain_bytes;
    }

    while (1)
    {
        HDL_G4_Flash_read(HDL_FLASH_ADDR_OF_SECTOR(sec), (uint8_t *)flash_buffer, HDL_FLASH_SECTOR_SIZE);
        current_sec_start_write_addr = address % HDL_FLASH_SECTOR_SIZE;
        for (uint32_t i = 0; i < current_sec_waiting_to_write_byte_num; i++)
        {
            flash_buffer[current_sec_start_write_addr + i] = data[i];
        }
        HDL_G4_Flash_erase_one_sector(sec);
        HDL_G4_Flash_write_one_sector_nocheck(sec, (uint8_t *)flash_buffer);

        size -= current_sec_waiting_to_write_byte_num;
        if (size == 0)
        {
            break;
        }

        address += current_sec_waiting_to_write_byte_num;
        data += current_sec_waiting_to_write_byte_num;
        if (size > HDL_FLASH_SECTOR_SIZE)
        {
            current_sec_waiting_to_write_byte_num = HDL_FLASH_SECTOR_SIZE;
        }
        else
        {
            current_sec_waiting_to_write_byte_num = size;
        }
        sec++;
    }
    return status;
}

#if defined(__CC_ARM)
#pragma push
#pragma O0
/**
 * @brief 从内部flash随机读取一定数量的数据到内存。
 *
 * @param address 从内部Flash读取数据的地址。
 * @param buf 指向待存放数据的指针。
 * @param size 需要读取数据的长度，单位字节。
 * @return int 返回实际读取到的字节数。
 */
int HDL_G4_Flash_read(uint32_t address, uint8_t *buf, uint32_t size)
{
    uint32_t pre_size = size / (HDL_FLASH_READ_BIT_WIDTHT / 8U) * (HDL_FLASH_READ_BIT_WIDTHT / 8U); //对齐小块的大小。

    switch (HDL_FLASH_READ_BIT_WIDTHT)
    {
    case HDL_FLASH_READ_8BIT:
        for (uint32_t i = 0; i < pre_size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
        break;
    case HDL_FLASH_READ_16BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 2)
        {
            *(uint16_t *)(buf + i) = *(__IO uint16_t *)(address + i);
        }
        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }

    break;
    case HDL_FLASH_READ_32BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 4)
        {
            *(uint32_t *)(buf + i) = *(__IO uint32_t *)(address + i);
        }
        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }
    break;
    case HDL_FLASH_READ_64BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 8)
        {
            *(uint64_t *)(buf + i) = *(__IO uint64_t *)(address + i);
        }

        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }

    break;
    case HDL_FLASH_READ_128BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 16)
        {
            *(uint64_t *)(buf + i) = *(__IO uint64_t *)(address + i);
            *(uint64_t *)(buf + i + 8) = *(__IO uint64_t *)(address + i + 8);
        }

        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }

    break;
    default:
        break;
    }
    return size;
}
#pragma pop
#elif defined(__GNUC__)
/**
 * @brief 从内部flash随机读取一定数量的数据到内存。
 *
 * @param address 从内部Flash读取数据的地址。
 * @param buf 指向待存放数据的指针。
 * @param size 需要读取数据的长度，单位字节。
 * @return int 返回实际读取到的字节数。
 */
int HDL_G4_Flash_read(uint32_t address, uint8_t *buf, uint32_t size) __attribute__((optnone))
{
    //为了加速读取数据，可能会一次读取64bit的数据，这样64bit的数据算作一个块，pre_size：有多少个块
    uint32_t pre_size = size / (HDL_FLASH_READ_BIT_WIDTHT / 8U) * (HDL_FLASH_READ_BIT_WIDTHT / 8U); //对齐小块的大小。
    //剩余的字节数按照一次8bit来读取。
    
    switch (HDL_FLASH_READ_BIT_WIDTHT)
    {
    case HDL_FLASH_READ_8BIT:
        for (uint32_t i = 0; i < pre_size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
        break;
    case HDL_FLASH_READ_16BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 2)
        {
            *(uint16_t *)(buf + i) = *(__IO uint16_t *)(address + i);
        }
        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }

    break;
    case HDL_FLASH_READ_32BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 4)
        {
            *(uint32_t *)(buf + i) = *(__IO uint32_t *)(address + i);
        }
        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }
    break;
    case HDL_FLASH_READ_64BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 8)
        {
            *(uint64_t *)(buf + i) = *(__IO uint64_t *)(address + i);
        }

        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }

    break;
    case HDL_FLASH_READ_128BIT:
    {
        for (uint32_t i = 0; i < pre_size; i += 16)
        {
            *(uint64_t *)(buf + i) = *(__IO uint64_t *)(address + i);
            *(uint64_t *)(buf + i + 8) = *(__IO uint64_t *)(address + i + 8);
        }

        for (uint32_t i = pre_size; i < size; i++)
        {
            buf[i] = *(__IO uint8_t *)(address + i);
        }
    }

    break;
    default:
        break;
    }
    return size;
}
#endif

/**
 * @brief 获取扇区首地址。
 *
 * @param sector
 * @return uint32_t 成功返回扇区编号，失败返回0xFFFFFFFFUL。
 */
uint32_t HDL_G4_Flash_get_address_by_sector(uint32_t sector)
{
    uint32_t addr = 0xFFFFFFFFUL;
    if (sector < HDL_FLASH_SECTOR_NB)
    {
        addr = HDL_FLASH_BASE_ADDR + sector * HDL_FLASH_SECTOR_SIZE;
    }
    return addr;
}

/**
 * @brief 擦除内部Flash指定的sector。
 *
 * @param sector 将要擦除的扇区的编号。
 * @return int 成功返回0，失败返回-1。
 */
int HDL_G4_Flash_erase_one_sector(uint32_t sector)
{
    int status = 0;

    HAL_FLASH_Unlock();
    /* wait BSY bit clear */
    FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

    FLASH_PageErase(sector % FLASH_PAGE_NB, sector / FLASH_PAGE_NB == 0 ? FLASH_BANK_1 : FLASH_BANK_2);
    /* Wait for last operation to be completed */
    status = (int)FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

    /* If the erase operation is completed, disable the PER Bit */
    CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));
    // uint32_t Page = sector;
    // /* Proceed to erase the page */
    // MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((Page & 0xFFU) << FLASH_CR_PNB_Pos));
    // SET_BIT(FLASH->CR, FLASH_CR_PER);
    // SET_BIT(FLASH->CR, FLASH_CR_STRT);

    HAL_FLASH_Lock();
    return status;
}

/**
 * @brief 擦除内部Flash指定的count数量的Sector。从sector开始，包括sector。
 *
 * @param sector 将要擦除的第一个扇区的编号。
 * @param count 要擦除的sector的数量。
 * @return int 成功返回0，失败返回-1。
 */
int HDL_G4_Flash_erase_sector(uint32_t sector, uint32_t count)
{
    int status = 0;
    uint32_t end_sector = sector + count;
    for (uint32_t i = sector; i < end_sector; i++)
    {
        status = HDL_G4_Flash_erase_one_sector(sector);
        if (status != 0)
        {
            break;
        }
    }
    return status;
}

/**
 * @brief _nocheck表示不检查擦除。
 * 
 * @param sector 
 * @param buf 
 * @return int 
 */
int HDL_G4_Flash_write_one_sector_nocheck(uint32_t sector, uint8_t *buf)
{
    int status = 0;
    uint32_t data_addr = (uint32_t)buf;
    uint32_t flash_addr = HDL_G4_Flash_get_address_by_sector(sector);
    uint64_t *data_ptr = (uint64_t *)buf;
    HAL_FLASH_Unlock();
    // STM32的Flash可以一次编程多字节数据，有三种模式。但是
    // FLASH_TYPEPROGRAM_FAST和FLASH_TYPEPROGRAM_FAST_AND_LAST只能使用MASSERASE的方式擦除。
    for (uint32_t idx = 0; idx < HDL_FLASH_SECTOR_SIZE / sizeof(uint64_t); idx++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flash_addr + idx * sizeof(uint64_t),
                                   data_ptr[idx]);
        if (status != HAL_OK)
        {
            status = -1;
            break;
        }
    }

    HAL_FLASH_Lock();
    return status;
}
