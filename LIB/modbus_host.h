/**
 * @file modbus_host.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 1.0
 * @date 2022-10-17
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#ifndef MOSBUS_HOST_H
#define MOSBUS_HOST_H

#include <stdint.h>
#include "HDL_G4_Uart.h"
#include "HDL_G4_CPU_Time.h"
#define MOD_BUS_DEBUG 1 // 0无调试信息，1有调试信息
#if MOD_BUS_DEBUG == 1
#include "log.h"
#endif // !MOD_BUS_DEBUG

/* 01H 读强制单线圈 */
/* 05H 写强制单线圈 */
#define REG_D01 0x0101
#define REG_D02 0x0102
#define REG_D03 0x0103
#define REG_D04 0x0104
#define REG_DXX REG_D04

/* 02H 读取输入状态 */
#define REG_T01 0x0201
#define REG_T02 0x0202
#define REG_T03 0x0203
#define REG_TXX REG_T03

/* 03H 读保持寄存器 */
/* 06H 写保持寄存器 */
/* 10H 写多个保存寄存器 */
#define REG_P01 0x0301
#define REG_P02 0x0302

/* 04H 读取输入寄存器(模拟信号) */
#define REG_A01 0x0401
#define REG_AXX REG_A01

/* RTU 应答代码 */
#define RSP_OK 0			  /* 成功 */
#define RSP_ERR_CMD 0x01	  /* 不支持的功能码 */
#define RSP_ERR_REG_ADDR 0x02 /* 寄存器地址错误 */
#define RSP_ERR_VALUE 0x03	  /* 数据值域错误 */
#define RSP_ERR_WRITE 0x04	  /* 写入失败 */

/* Modbus RTU 地址*/

#define MODBUS_RTU_BROADCAST_ADDR 0x00 /*主机发送广播*/
#define MODBUS_RTU_ANY_ADDRESS 0x00	   /*用于接收时匹配过滤地址时指示接收任意地址0-255*/

#define H_RX_BUF_SIZE 64
#define H_TX_BUF_SIZE 128
struct tagModbusInstance_t;

typedef struct tagVAR_T
{
	/* 03H 06H 读写保持寄存器 */
	uint16_t P[4];
	uint8_t len;		  // 收到数据的字节数
	uint8_t register_num; // 收到数据等同于多少个寄存器数量
	/* 02H 读写离散输入寄存器 */
	uint16_t T01;
	uint16_t T02;
	uint16_t T03;

	/* 04H 读取模拟量寄存器 */
	uint16_t A01;

	/* 01H 05H 读写单个强制线圈 */
	uint16_t D01;
	uint16_t D02;
	uint16_t D03;
	uint16_t D04;
} ModbusRTU_Res_t;

/*传输结果标志位：其逻辑图见《光伏RTU Modbus RTU传输协议实现》中所示。
因为一个总线一次只有一个传输，所以只有一个标志位。
该标志位在每次重新发送命令时清除为0，每次收到正确应答化为1，传输错误大于1：传输超时为2，帧错误为3。
*/
typedef enum
{
	MODBUS_RTU_TRANS_RES_NONE = 0, // 至少每次重新发送Modbus命令时候的状态，不属于Modbus传输,表示没有事件
	MODBUS_RTU_TRANS_REV_ACK = 1,
	MODBUS_RTU_TRANS_ERR_TRANS_TIMEOUT = 2,
	MODBUS_RTU_TRANS_ERR_REV_FRME = 3,

	MODBUS_RTU_TRANS_ANY_EVENT, // 不属于Modbus传输,表示匹配任何事件
} ModbusRTU_TransEvent_t;

#define MODBUS_RTU_TRANS_STATE_IDLE 0
#define MODBUS_RTU_TRANS_STATE_TRANSMITING 1
// 等待处理意味着传输到了需要处理数据或者错误的阶段，这些错误和数据称为事件。
#define MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE 2

#define MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, state) ((hmodbusRTU)->transStageState = (state))
#define MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU) ((hmodbusRTU)->transStageState = MODBUS_RTU_TRANS_STATE_IDLE)

// 1：使用modbus_rtu_host_uart_msg_fatch()通过轮询方法从串口消息队列获取数据，
// 0：使用回调方法，直接注册接收字符函数到串口中断中
#define USEING_POLL_MODE_TO_GET_CHAR_STREAM 0
struct tagModbusInstance_t;
typedef struct tagModbusInstance_t ModbusRTUInstance_t;

typedef void (*ModbusInnerTimeoutEventHandler_t)(ModbusRTUInstance_t *hmodbusRTU);

struct tagModbusInstance_t
{
	uint8_t RxBuf[H_RX_BUF_SIZE];
	uint8_t _RxCount; // 每次接收到数据后更新，直到内部处理完成接收的数据后重置。
	uint8_t RxCount;

	uint8_t TxBuf[H_TX_BUF_SIZE];
	uint8_t _TxCount;

	uint16_t RegAddr; /* 保存主机发送的寄存器首地址 */
	uint8_t RegNum;	  /* 保存主机发送的寄存器个数 */

	/*传输事件(结果)标志位：其逻辑图见《光伏RTU Modbus RTU传输协议实现》中所示。
	因为一个总线一次只有一个传输，所以只有一个标志位。
	该标志位在每次重新发送命令或者清除全部事件时清除为0，每次收到正确应答化为1，传输错误大于1：传输超时为2，帧错误为3。
	即这个传输的结果会保留到处理完成接收到的数据。

	传输事件不用于控制传输本身，只是显示一下当前传输完成等待处理的事件是什么。
	*/
	ModbusRTU_TransEvent_t transEvent;

	/*传输步骤状态标志位：0表示该modbus总线当前空闲（本次传输的的处理完成），1表示正处于一个传输周期，2表示本次传输需要处理。
	这是一个关键变量，尤其是可能一部分在中断中执行，一部分在用户线程中执行，建议更换为原子变量。
	传输正常完成需要处理数据（正常数据和错误报告），传输超时需要做超时处理。
	该标志位在每次传输启动时置位1，每次超时或者收到回复时变为待处理2。通过调用读取方法清除或者手动清除为0。
	如果上次的传输没有及时处理，没有去清除标志位，那么就无法启动下一次传输。*/
	uint8_t transStageState;

	uint8_t slaveAddr;			// 本次传输发送的从机地址,发送命令时设置
	uint8_t func_code;			// 本次传输发送的功能码,发送命令时设置
	uint8_t respons_func_code;	// 本次传输收到的功能码,收到命令时设置
	uint8_t gTimeOutT35Flag;	// 接收字符时间超过3.5个字符时间超时标志位
	uint8_t enabled;			// Mobus RTU 是否初始化完成（未使能）
	uint32_t lastRevByteUsTick; // 上次接收到字符的微秒时间戳
	uint32_t usTimeOut35;		// 3.5字符微秒(前缀us)延迟时间
	uint32_t usTimeOut15;		// 1.5字符间隔异常微秒(前缀us)时间
	/*一次传输开始时刻，在每次发送命令时更新。例如发送一个读取命令到成功接收到对应消息算作一次传输。*/
	uint32_t tickstart;
	ModbusRTU_Res_t result;

	uint32_t baud;
	COMID_t com;
	void (*gpio_init)(void);
	UartWriteOverCallback_t release_bus;
	UartWriteOverCallback_t get_bus;
	UartReceiveCharCallback_t receive_char_callback;
	CPU_Time_Callback_t t35_timeout_callback;
	uint32_t TIMEOUT; /* 接收命令超时时间, 单位ms */
	ModbusInnerTimeoutEventHandler_t innerTimeoutEventHandler;
};

uint8_t modbus_rtu_host_read_01H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_read_02H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_read_03H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_read_04H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_write_05H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value);
uint8_t modbus_rtu_host_write_06H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value);
uint8_t modbus_rtu_host_write_10H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint8_t _num, uint8_t *_buf);

uint8_t modbus_rtu_host_read_cmd_01H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_read_cmd_02H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_read_cmd_03H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_read_cmd_04H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num);
uint8_t modbus_rtu_host_write_cmd_05H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value);
uint8_t modbus_rtu_host_write_cmd_06H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value);
uint8_t modbus_rtu_host_write_cmd_10H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint8_t _num, uint8_t *_buf);

/**
 * @brief 返回modbus本次传输目标从设备的地址
 *
 */
#define modbus_rtu_host_trans_addr(hmodbusRTU) ((hmodbusRTU)->slaveAddr)

void modbus_rtu_host_init(ModbusRTUInstance_t *hmodbusRTU, int baud, char parity, int data_bit);
void modbus_rtu_handler(void);
uint8_t modbus_rtu_host_have_trans_event(ModbusRTUInstance_t *hmodbusRTU);

uint8_t modbus_rtu_host_match_event(ModbusRTUInstance_t *hmodbusRTU, ModbusRTU_TransEvent_t event, uint8_t slave_addr, uint8_t func_code);

/**
 * @brief 匹配回复事件，根据MODBUS_RTU_TRANS_REV_ACK事件、本次传输的命令的功能码。
 *
 * @param hmodbusRTU
 * @param event 事件。
 * @param cmd 功能码。
 * @return uint8_t 1匹配成功，0匹配失败。
 */
#define modbus_rtu_host_match_ack_event(hmodbusRTU, func_code, slave_addr) modbus_rtu_host_match_event((hmodbusRTU), MODBUS_RTU_TRANS_REV_ACK, (slave_addr), (func_code))
#define modbus_rtu_host_clear_current_trans_event modbus_rtu_host_clear_all_trans_event
void modbus_rtu_host_clear_all_trans_event(ModbusRTUInstance_t *hmodbusRTU);

/**
 * @brief 从modbus rtu收到的数据中按照uint16格式读取num个uint16数据。
 *
 * @param hmodbusRTU
 * @param pBuf 存放读取数据的缓存的指针。
 * @param Num 返回实际实际读取了多少个uint16类型的数据。
 */
int modbus_rtu_get_data_uint16(ModbusRTUInstance_t *hmodbusRTU, uint16_t *pBuf, int Num);

/**
 * @brief 从modbus rtu收到的数据中按照int16格式读取num个int16数据。
 *
 * @param hmodbusRTU
 * @param pBuf 存放读取数据的缓存的指针。
 * @param Num 返回实际实际读取了多少个int16类型的数据。
 */
int modbus_rtu_get_data_int16(ModbusRTUInstance_t *hmodbusRTU, int16_t *pBuf, int Num);

extern ModbusRTUInstance_t *hModbusRTU3;
extern ModbusRTUInstance_t *hModbusRTU4;
extern ModbusRTUInstance_t *hModbusRTU5;

/*************************Document****************************/
/*
对于从机主动发送的消息不予理会，这破坏了传输由请求-回应构成的规则，也不符号modbus的要求。
*/
/*************************Document End************************/
#endif // !MOSBUS_HOST_H
