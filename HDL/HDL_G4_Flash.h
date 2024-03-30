/**
 * @file HDL_G4_Flash.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-12
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#ifndef HDL_G4_FLASH_H
#define HDL_G4_FLASH_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "main.h"
#include "stm32g4xx_hal_flash.h"

/*
对于有两个BANK的情况，BANK1开始的page作为第0个扇区，BANK2的起始扇区号就是
BANK1结束扇区号+1.
*/
/**
 * @brief  Flash info.
 *
 */
#define HDL_FLASH_SECTOR_SIZE FLASH_PAGE_SIZE
#define HDL_FLASH_SIZE FLASH_SIZE
#define HDL_FLASH_SECTOR_NB (FLASH_PAGE_NB * 2) // 两个BANK，一个BANK128个page
#define HDL_FLASH_BASE_ADDR FLASH_BASE
#define HDL_FLASH_END_ADDR (HDL_FLASH_BASE_ADDR + HDL_FLASH_SIZE - 1)

#define HDL_FLASH_READ_8BIT (8UL)
#define HDL_FLASH_READ_16BIT (16UL)
#define HDL_FLASH_READ_32BIT (32UL)
#define HDL_FLASH_READ_64BIT (64UL)
#define HDL_FLASH_READ_128BIT (128UL)

/*In stm32 mcu, the flash read bit width alaways 32bit*/
#define HDL_FLASH_READ_BIT_WIDTHT HDL_FLASH_READ_64BIT
/*
Flash编程并行位数。64UL按照双字（2*32bit）写入。
 */
#define HDL_FLASH_PROGRAMM_BIT_WIDTHT 64UL
/*
快速编程时每次写入的字节数量。FLASH_TYPEPROGRAM_FAST->
Fast program a 32 row double-word (64-bit) at a specified address.
*/
#define HDL_FLASH_WRITE_SIZE_PER_PROG (8UL)
/**
 * @brief some define for flash operation
 *
 */
#define HDL_FLASH_READ_SIZE_ALIGN(size) (HDL_ALIGN(size, HDL_FLASH_READ_BIT_WIDTHT / 8))
#define HDL_FLASH_PROGRAMM_BYTE_WIDTHT (HDL_FLASH_PROGRAMM_BIT_WIDTHT / 8)
/**
 * @brief 得到扇区对应的首地址。
 *
 */
#define HDL_FLASH_ADDR_OF_SECTOR(sector) (HDL_FLASH_BASE_ADDR + (sector)*HDL_FLASH_SECTOR_SIZE)
#define HDL_SECTOR_OF_ADDRESS(address) (((address)-HDL_FLASH_BASE_ADDR) / HDL_FLASH_SECTOR_SIZE)
#define FLASH_LAST_SECTOR_INDEX (HDL_FLASH_SECTOR_NB - 1)

    int HDL_G4_Flash_init();
    int HDL_G4_Flash_write(uint32_t address, uint8_t *data, uint32_t size);
    int HDL_G4_Flash_read(uint32_t address, uint8_t *buf, uint32_t size);
#ifdef __cplusplus
}
#endif
#endif //! HDL_G4_FLASH_H
