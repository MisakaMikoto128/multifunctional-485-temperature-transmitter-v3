/**
* @file HDL_IIC.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-29
* @last modified 2024-03-29
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef HDL_IIC_H
#define HDL_IIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// IIC所有操作函数
void IIC_Init(void);                      // 初始化IIC的IO口
void IIC_Start(void);                     // 发送IIC开始信号
void IIC_Stop(void);                      // 发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);          // IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack); // IIC读取一个字节
uint8_t IIC_Wait_Ack(void);               // IIC等待ACK信号
void IIC_Ack(void);                       // IIC发送ACK信号
void IIC_NAck(void);                      // IIC不发送ACK信号

typedef enum {
    IIC_1 = 0,
    IIC_NUM,
} IIC_ID_t;

void HDL_IIC_Init(IIC_ID_t iicID, uint32_t speed, uint8_t addrLength, uint16_t ownAddr);

void HDL_IIC_DeInit(IIC_ID_t iicID);

size_t HDL_IIC_Write(IIC_ID_t iicID, uint16_t addr, const uint8_t *data, size_t size);

size_t HDL_IIC_Read(IIC_ID_t iicID, uint16_t addr, uint8_t *buf, size_t size);

size_t HDL_IIC_Mem_Write(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, const uint8_t *data, size_t size);

size_t HDL_IIC_Mem_Read(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, uint8_t *buf, size_t size);

size_t HDL_IIC_Write_DMA(IIC_ID_t iicID, uint16_t addr, const uint8_t *data, size_t size);

size_t HDL_IIC_Read_DMA(IIC_ID_t iicID, uint16_t addr, uint8_t *buf, size_t size);

size_t HDL_IIC_Mem_Write_DMA(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, const uint8_t *data, size_t size);

size_t HDL_IIC_Mem_Read_DMA(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif
#endif //!HDL_IIC_H
