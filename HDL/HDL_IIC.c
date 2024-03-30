#include "HDL_IIC.h"
#include "main.h"
#include "HDL_CPU_Time.h"
#include "log.h"
#include "i2c.h"

#define USING_HAL_I2C         0

#define IIC_SDA_PIN           LL_GPIO_PIN_7
#define IIC_SCL_PIN           LL_GPIO_PIN_15

#define IIC_SDA_PORT          GPIOB
#define IIC_SCL_PORT          GPIOA

#define IIC_SDA_PORT_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB)
#define IIC_SCL_PORT_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA)

#define IIC_SCL_1             LL_GPIO_SetOutputPin(IIC_SCL_PORT, IIC_SCL_PIN)   /* SCL = 1 */
#define IIC_SCL_0             LL_GPIO_ResetOutputPin(IIC_SCL_PORT, IIC_SCL_PIN) /* SCL = 0 */

#define IIC_SDA_1             LL_GPIO_SetOutputPin(IIC_SDA_PORT, IIC_SDA_PIN)   /* SDA = 1 */
#define IIC_SDA_0             LL_GPIO_ResetOutputPin(IIC_SDA_PORT, IIC_SDA_PIN) /* SDA = 0 */

#define IIC_READ_SDA          LL_GPIO_IsInputPinSet(IIC_SDA_PORT, IIC_SDA_PIN) /* SDA输入 */

#define HDL_CPU_Time_DelayUs  HDL_G4_CPU_Time_DelayUs
// IIC所有操作函数
void IIC_Init(void);                      // 初始化IIC的IO口
void IIC_Start(void);                     // 发送IIC开始信号
void IIC_Stop(void);                      // 发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);          // IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack); // IIC读取一个字节
uint8_t IIC_Wait_Ack(void);               // IIC等待ACK信号
void IIC_Ack(void);                       // IIC发送ACK信号
void IIC_NAck(void);                      // IIC不发送ACK信号

void IIC_Init()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    IIC_SDA_PORT_CLK_EN();
    IIC_SCL_PORT_CLK_EN();

    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;

    GPIO_InitStruct.Pin = IIC_SDA_PIN;
    LL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IIC_SCL_PIN;
    LL_GPIO_Init(IIC_SCL_PORT, &GPIO_InitStruct);
}

void SDA_IN()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin                 = IIC_SDA_PIN;
    GPIO_InitStruct.Mode                = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull                = LL_GPIO_PULL_UP;
    LL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

void SDA_OUT()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin                 = IIC_SDA_PIN;
    GPIO_InitStruct.Mode                = LL_GPIO_MODE_OUTPUT; // 推挽输出
    GPIO_InitStruct.OutputType          = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed               = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
    IIC_SDA_1;
}

// 产生IIC起始信号
void IIC_Start(void)
{
    SDA_OUT();
    IIC_SDA_1;
    IIC_SCL_1;
    HDL_CPU_Time_DelayUs(4);
    IIC_SDA_0;
    HDL_CPU_Time_DelayUs(4);
    IIC_SCL_0;
}

// 产生IIC停止信号
void IIC_Stop(void)
{
    SDA_OUT();
    IIC_SCL_0;
    IIC_SDA_0;
    HDL_CPU_Time_DelayUs(4);
    IIC_SCL_1;
    IIC_SDA_1;
    HDL_CPU_Time_DelayUs(4);
}

// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
    uint8_t errCount = 0;
    SDA_IN();
    IIC_SDA_1;
    HDL_CPU_Time_DelayUs(2);
    IIC_SCL_1;
    HDL_CPU_Time_DelayUs(2);
    while (IIC_READ_SDA) {
        errCount++;
        if (errCount > 250) {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_0;

    return 0;
}
// 产生ACK应答
void IIC_Ack(void)
{
    IIC_SCL_0;
    SDA_OUT();
    IIC_SDA_0;
    HDL_CPU_Time_DelayUs(2);
    IIC_SCL_1;
    HDL_CPU_Time_DelayUs(2);
    IIC_SCL_0;
}
// 不产生ACK应答
void IIC_NAck(void)
{
    IIC_SCL_0;
    SDA_OUT();
    IIC_SDA_1;
    HDL_CPU_Time_DelayUs(2);
    IIC_SCL_1;
    HDL_CPU_Time_DelayUs(2);
    IIC_SCL_0;
}
// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    SDA_OUT();
    IIC_SCL_0;
    for (t = 0; t < 8; t++) {
        if ((txd & 0x80) >> 7)
            IIC_SDA_1;
        else
            IIC_SDA_0;
        txd <<= 1;
        HDL_CPU_Time_DelayUs(2);
        IIC_SCL_1;
        HDL_CPU_Time_DelayUs(2);
        IIC_SCL_0;
        HDL_CPU_Time_DelayUs(2);
    }
}
// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
uint8_t IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN(); // SDA设置为输入
    for (i = 0; i < 8; i++) {
        IIC_SCL_0;
        HDL_CPU_Time_DelayUs(2);
        IIC_SCL_1;
        receive <<= 1;
        if (IIC_READ_SDA)
            receive++;
        HDL_CPU_Time_DelayUs(1);
    }
    if (!ack)
        IIC_NAck(); // 发送nACK
    else
        IIC_Ack(); // 发送ACK
    return receive;
}

void HDL_IIC_Init(IIC_ID_t iicID, uint32_t speed, uint8_t addrLength, uint16_t ownAddr)
{
#if USING_HAL_I2C
    MX_I2C1_Init();
#else
    IIC_Init();
#endif
}
/**
 * @brief IIC取消初始化。
 *
 * @param iicID IIC编号。
 */
void HDL_IIC_DeInit(IIC_ID_t iicID)
{
    switch (iicID) {
        case IIC_1:
            HAL_I2C_DeInit(&hi2c1);
            break;
        default:
            break;
    }
}

/**
 * @brief IIC发送数据。
 *
 * @param iicID IIC编号。
 * @param addr 目标设备地址。
 * @param data 数据指针。
 * @param size 数据长度。
 * @return size_t 0: 失败，> 1 实际写入的数据长度。
 */
size_t HDL_IIC_Write(IIC_ID_t iicID, uint16_t addr, const uint8_t *data, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1: {
#if USING_HAL_I2C
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Master_Transmit(&hi2c1, addr << 1, (uint8_t *)data, size, 0xFFFF);
#else
            IIC_Start();
            IIC_Send_Byte(addr << 1 | 0x00);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            for (size_t i = 0; i < size; i++) {
                IIC_Send_Byte(data[i]);
                if (IIC_Wait_Ack()) {
                    IIC_Stop();
                    return i;
                }
            }
            IIC_Stop();
#endif
        } break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief IIC读取数据。
 *
 * @param iicID IIC编号。
 * @param addr 目标设备地址。
 * @param buf 数据指针。
 * @param size 数据长度。
 * @return size_t 0: 失败，> 1 实际读取的数据长度。
 */
size_t HDL_IIC_Read(IIC_ID_t iicID, uint16_t addr, uint8_t *buf, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1: {
#if USING_HAL_I2C
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Master_Receive(&hi2c1, addr << 1, buf, size, 0xFFFF);
#else
            IIC_Start();
            IIC_Send_Byte(addr << 1 | 0x01);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            for (size_t i = 0; i < size; i++) {
                buf[i] = IIC_Read_Byte(i == size - 1);
            }
            IIC_Stop();
#endif
        } break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief Write an amount of data in blocking mode to a specific memory address.
 *
 * @param iicID IIC identifier.
 * @param addr Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param memAddr Internal memory address
 * @param memAddrLength Size of internal memory address.8 or 16
 * @param data Pointer to data buffer
 * @param size Amount of data to be sent
 * @return size_t 0: 失败，> 1 实际写入的数据长度。
 */
size_t HDL_IIC_Mem_Write(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, const uint8_t *data, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1: {
#if USING_HAL_I2C
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Mem_Write(&hi2c1, addr, memAddr, memAddrLength == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, size, 0xFFFF);
#else
            IIC_Start();
            IIC_Send_Byte(addr << 1 | 0x00);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            if (memAddrLength == 16) {
                IIC_Send_Byte(memAddr >> 8);
                if (IIC_Wait_Ack()) {
                    IIC_Stop();
                    return 0;
                }
            }

            IIC_Send_Byte(memAddr & 0xFF);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            for (size_t i = 0; i < size; i++) {
                IIC_Send_Byte(data[i]);
                if (IIC_Wait_Ack()) {
                    IIC_Stop();
                    return i;
                }
            }
            IIC_Stop();

        } break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief Read an amount of data in blocking mode from a specific memory address
 *
 * @param iicID IIC identifier.
 * @param addr Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param memAddr Internal memory address
 * @param memAddrLength Size of internal memory address
 * @param buf Pointer to data buffer
 * @param size Amount of data to be sent
 * @return size_t 0: 失败，> 1 实际读取的数据长度。
 */
size_t HDL_IIC_Mem_Read(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, uint8_t *buf, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1: {
#if USING_HAL_I2C
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Mem_Write(&hi2c1, addr, memAddr, memAddrLength == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, buf, size, 0xFFFF);
#else
            IIC_Start();
            IIC_Send_Byte(addr << 1 | 0x00);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            if (memAddrLength == 16) {
                IIC_Send_Byte(memAddr >> 8);
                if (IIC_Wait_Ack()) {
                    IIC_Stop();
                    return 0;
                }
            }

            IIC_Send_Byte(memAddr & 0xFF);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            IIC_Start();
            IIC_Send_Byte(addr << 1 | 0x01);
            if (IIC_Wait_Ack()) {
                IIC_Stop();
                return 0;
            }
            for (size_t i = 0; i < size; i++) {
                buf[i] = IIC_Read_Byte(i == size - 1);
            }
            IIC_Stop();
#endif
        } break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief IIC发送数据。
 *
 * @param iicID IIC编号。
 * @param addr 目标设备地址。
 * @param data 数据指针。
 * @param size 数据长度。
 * @return size_t 0: 失败，> 1 实际写入的数据长度。
 */
size_t HDL_IIC_Write_DMA(IIC_ID_t iicID, uint16_t addr, const uint8_t *data, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1:
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Master_Transmit_DMA(&hi2c1, addr, (uint8_t *)data, size);
            break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief IIC读取数据。
 *
 * @param iicID IIC编号。
 * @param addr 目标设备地址。
 * @param buf 数据指针。
 * @param size 数据长度。
 * @return size_t 0: 失败，> 1 实际读取的数据长度。
 */
size_t HDL_IIC_Read_DMA(IIC_ID_t iicID, uint16_t addr, uint8_t *buf, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1:
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Master_Receive(&hi2c1, addr, buf, size, 0xFFFF);
            break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief Write an amount of data in blocking mode to a specific memory address.
 *
 * @param iicID IIC identifier.
 * @param addr Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param memAddr Internal memory address
 * @param memAddrLength Size of internal memory address.8 or 16
 * @param data Pointer to data buffer
 * @param size Amount of data to be sent
 * @return size_t 0: 失败，> 1 实际写入的数据长度。
 */
size_t HDL_IIC_Mem_Write_DMA(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, const uint8_t *data, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1:
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Mem_Write_DMA(&hi2c1, addr, memAddr, memAddrLength == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, size);
            break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

/**
 * @brief Read an amount of data in blocking mode from a specific memory address
 *
 * @param iicID IIC identifier.
 * @param addr Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param memAddr Internal memory address
 * @param memAddrLength Size of internal memory address
 * @param buf Pointer to data buffer
 * @param size Amount of data to be sent
 * @return size_t 0: 失败，> 1 实际读取的数据长度。
 */
size_t HDL_IIC_Mem_Read_DMA(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr, uint8_t memAddrLength, uint8_t *buf, size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    switch (iicID) {
        case IIC_1:
            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
            status = HAL_I2C_Mem_Write_DMA(&hi2c1, addr, memAddr, memAddrLength == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, buf, size);
            break;
        default:
            break;
    }
    return status == HAL_OK ? size : 0;
}

I2C_HandleTypeDef *HDL_IIC_GetHALHandle(IIC_ID_t iicID)
{
    switch (iicID) {
        case IIC_1:
            return &hi2c1;
        default:
            return NULL;
    }
}