/**
 * @file HDL_G4_Flash_test.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-10-12
 * 
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 * 
 */
#ifndef HDL_G4_FLASH_TEST_H
#define HDL_G4_FLASH_TEST_H
#include "HDL_G4_Flash.h"
void Flash_test();
void Flash_inner_fun_test();

uint32_t HDL_G4_Flash_get_address_by_sector(uint32_t sector);
int HDL_G4_Flash_write_one_sector_nocheck(uint32_t sector, uint8_t *buf);
int HDL_G4_Flash_erase_one_sector(uint32_t sector);
int HDL_G4_Flash_erase_sector(uint32_t sector, uint32_t count);

#endif // !HDL_G4_FLASH_TEST_H
