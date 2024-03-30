#include "modbus_test.h"
#include "HDL_G4_CPU_Time.h"
#include "log.h"
#include "main.h"
#include "test.h"


//TODO:修改了modbus的接口后有些测试已经不能用来
void modbus_rtu_host_test()
{
    HDL_G4_CPU_Time_Init();
    modbus_rtu_host_init(hModbusRTU3, 115200, 'N', 8);

    ulog_init_user();
    uint16_t var = 1;
		PeriodREC_t rec1 = 0;
    while (1)
    {
        modbus_rtu_handler();
        // 一秒执行一次
        if (period_query_user(&rec1, 500))
        {
            var++;
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x01, 0x0001, 1) == 1)
            {
                Debug_Printf("Read 03H reg %x receive %d bytes %d\r\n", 0x0001, hModbusRTU3->RxCount, 1);
                for (size_t i = 0; i < hModbusRTU3->RxCount; i++)
                {
                    Debug_Printf("%#x ", hModbusRTU3->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
            }

            if (modbus_rtu_host_write_06H(hModbusRTU3, 0x01, 0x0001, var) == 1)
            {
                Debug_Printf("Write 06H %x var %d\r\n", 0x0001, var);
                for (size_t i = 0; i < hModbusRTU3->RxCount; i++)
                {
                    Debug_Printf("%#x ", hModbusRTU3->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
            }

            if (modbus_rtu_host_write_05H(hModbusRTU3, 0x01, 0x0001, var % 2 == 0 ? 0 : 0xFF00) == 1)
            {
                Debug_Printf("Write 05H %x var %d\r\n", 0x0001, var % 2 == 0 ? 0 : 0xFF);
                for (size_t i = 0; i < hModbusRTU3->RxCount; i++)
                {
                    Debug_Printf("%#x ", hModbusRTU3->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
            }

            if (modbus_rtu_host_read_01H(hModbusRTU3, 0x01, 0x0001, 1) == 1)
            {
                Debug_Printf("Read 01H reg %x RxCount %d bytes %d\r\n", 0x0001, hModbusRTU3->RxCount, 1);
                for (size_t i = 0; i < hModbusRTU3->RxCount; i++)
                {
                    Debug_Printf("%#x ", hModbusRTU3->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
            }
        }
    }
}

/**
 * @brief 测试命令和处理分离方法。
 *
 */
void modbus_rtu_host_test2()
{
    HDL_G4_CPU_Time_Init();
    modbus_rtu_host_init(hModbusRTU3, 115200, 'N', 8);
    uint32_t cpu_tick = HDL_G4_CPU_Time_GetTick();
    ulog_init_user();
    uint16_t var = 1;
    while (1)
    {
        modbus_rtu_handler();
        // 一秒执行一次
        if ((HDL_G4_CPU_Time_GetTick() - cpu_tick) > 500)
        {
            cpu_tick = HDL_G4_CPU_Time_GetTick();
            var++;
            if (modbus_rtu_host_read_cmd_01H(hModbusRTU3, 0x01, 0x0001, 1))
            {
                Debug_Printf("Send 01H successfully\r\n");
            }

            if (modbus_rtu_host_have_trans_event(hModbusRTU3))
            {
                Debug_Printf("Read 01H reg %x RxCount %d bytes %d\r\n", 0x0001, hModbusRTU3->RxCount, 1);
                for (size_t i = 0; i < hModbusRTU3->RxCount; i++)
                {
                    Debug_Printf("%#x ", hModbusRTU3->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
                modbus_rtu_host_clear_current_trans_event(hModbusRTU3);
            }
        }
    }
}

/**
 * @brief RTU项目用到的RS485 3通信传感器测试
 *
 */
void modbus_rtu_host_rs485_3_test()
{
    HDL_G4_CPU_Time_Init();
    modbus_rtu_host_init(hModbusRTU3, 9600, 'N', 8);
    uint32_t cpu_tick = HDL_G4_CPU_Time_GetTick();
    ulog_init_user();

    while (1)
    {
        modbus_rtu_handler();
        // 500ms执行一次
        if ((HDL_G4_CPU_Time_GetTick() - cpu_tick) > 500)
        {
            cpu_tick = HDL_G4_CPU_Time_GetTick();

            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x03, 0x0000, 1) == 1)
            {
                Debug_Printf("Read 03H reg %x RxCount %d bytes %d\r\n", 0x0001, hModbusRTU3->RxCount, 1);
                for (size_t i = 0; i < hModbusRTU3->RxCount; i++)
                {
                    Debug_Printf("%#02x ", hModbusRTU3->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
            }
        }
    }
}

/**
 * @brief RTU项目用到的RS485 4通信传感器测试
 *
 */
void modbus_rtu_host_rs485_4_test()
{
    HDL_G4_CPU_Time_Init();
    modbus_rtu_host_init(hModbusRTU4, 9600, 'N', 8);
    uint32_t cpu_tick = HDL_G4_CPU_Time_GetTick();
    ulog_init_user();

    while (1)
    {
        modbus_rtu_handler();
        // 一秒执行一次
        if ((HDL_G4_CPU_Time_GetTick() - cpu_tick) > 500)
        {
            cpu_tick = HDL_G4_CPU_Time_GetTick();

            if (modbus_rtu_host_read_03H(hModbusRTU4, 0x50, 0x003D, 3) == 1)
            {
                Debug_Printf("Read 03H reg %x RxCount %d bytes %d\r\n", 0x003D, hModbusRTU4->RxCount, 1);
                for (size_t i = 0; i < hModbusRTU4->RxCount; i++)
                {
                    Debug_Printf("%#02x ", hModbusRTU4->RxBuf[i]);
                }
                Debug_Printf("\r\n\r\n");
            }
        }
    }
}

/**
 * @brief RTU项目用到的RS485 5通信传感器测试
 *
 */
void modbus_rtu_host_sensor_test()
{

    // 初始化RS485 3
    modbus_rtu_host_init(hModbusRTU3, 9600, 'N', 8);
    // 初始化RS485 4
    modbus_rtu_host_init(hModbusRTU4, 9600, 'N', 8);
    // 初始化RS485 5
    modbus_rtu_host_init(hModbusRTU5, 9600, 'N', 8);

    HDL_G4_CPU_Time_Init();
    ulog_init_user();

    uint32_t cpu_tick = HDL_G4_CPU_Time_GetTick();
    while (1)
    {
        modbus_rtu_handler();
        // 500ms执行一次
        if ((HDL_G4_CPU_Time_GetTick() - cpu_tick) > 500)
        {
            cpu_tick = HDL_G4_CPU_Time_GetTick();

            // 读取光照（#3）传感器,这个传感器只要485总线不空闲它就会处以一种状态，需要总线空闲100ms后才能读取。
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x03, 0x0000, 1) == 1)
            {
                Debug_Printf("收到寄存器值：%#02x，光照：%dW/m^2", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0]);
                Debug_Printf("\r\n");
            }

            // 读取风速（#1）传感器
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x01, 0x0000, 1) == 1)
            {
                Debug_Printf("收到寄存器值：%#02x，风速：%.2fm/s", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
            }

            // 读取风向（#2）传感器
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x02, 0x0000, 1) == 1)
            {
                Debug_Printf("收到寄存器值：%#02x，风向：%.2f°", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
            }

            // 读取锚索计（#4）传感器
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x04, 0x0000, 1) == 1)
            {
                Debug_Printf("收到寄存器值：%#02x，锚索计4：%.2fHz", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
            }

            // 读取锚索计（#8）传感器
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x08, 0x0000, 1) == 1)
            {
                Debug_Printf("收到寄存器值：%#02x，锚索计8：%.2fHz", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
            }

            // 读取锚索计（#6）传感器
            if (modbus_rtu_host_read_03H(hModbusRTU3, 0x06, 0x0000, 1) == 1)
            {
                Debug_Printf("收到寄存器值：%#02x，锚索计6：%.2fHz", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
            }

            // 读取姿态（#50）传感器
            if (modbus_rtu_host_read_03H(hModbusRTU4, 0x50, 0x003D, 3) == 1)
            {
                Debug_Printf("收到寄存器值：X %#04x,Y %#04x,Z %#04x,，姿态X：%.2f,Y:%.2f,Z:%.2f",
                             hModbusRTU4->result.P[0], hModbusRTU4->result.P[1], hModbusRTU4->result.P[2],
                             hModbusRTU4->result.P[0] / 32768.0f * 180,
                             hModbusRTU4->result.P[1] / 32768.0f * 180,
                             hModbusRTU4->result.P[2] / 32768.0f * 180);
                Debug_Printf("\r\n");
            }
        }
    }
}

/**
 * @brief RTU项目用到的RS485 5通信传感器测试。测试命令和处理分离方法。
 *
 */
void modbus_rtu_host_sensor_test2()
{
    // 初始化RS485 3
    modbus_rtu_host_init(hModbusRTU3, 9600, 'N', 8);
    // 初始化RS485 4
    modbus_rtu_host_init(hModbusRTU4, 9600, 'N', 8);
    // 初始化RS485 5
    modbus_rtu_host_init(hModbusRTU5, 9600, 'N', 8);

    HDL_G4_CPU_Time_Init();
    ulog_init_user();

    uint32_t cpu_tick = HDL_G4_CPU_Time_GetTick();

    LoopFrequencyTest_t loop_frq_test = {
        .measure_time = 1000, // 测试1秒钟
        // 其他成员默认初始化为0.
    };

    uint8_t modbusRTU3_devid = 0;
#define MODBUSRTU3_DEV_NUM 6
    while (1)
    {

        modbus_rtu_handler();

        test_LoopFrequencyTest_handler(&loop_frq_test);

        if (test_LoopFrequencyTest_readable(&loop_frq_test))
        {
            test_LoopFrequencyTest_show(&loop_frq_test, "Main");
            test_LoopFrequencyTest_reset(&loop_frq_test);
        }

        // 100ms执行一次
        if ((HDL_G4_CPU_Time_GetTick() - cpu_tick) > 50)
        {
            cpu_tick = HDL_G4_CPU_Time_GetTick();

            switch (modbusRTU3_devid)
            {
            case 0:
                if (modbus_rtu_host_read_cmd_03H(hModbusRTU3, 0x01, 0x0000, 1) == 0)
                {
                    Debug_Printf("Send 01H error\r\n");
                }
                break;
            case 1:
                if (modbus_rtu_host_read_cmd_03H(hModbusRTU3, 0x02, 0x0000, 1) == 0)
                {
                    Debug_Printf("Send 01H error\r\n");
                }
                break;
            case 2:
                if (modbus_rtu_host_read_cmd_03H(hModbusRTU3, 0x03, 0x0000, 1) == 0)
                {
                    Debug_Printf("Send 01H error\r\n");
                }
                break;
            case 3:
                if (modbus_rtu_host_read_cmd_03H(hModbusRTU3, 0x04, 0x0000, 1) == 0)
                {
                    Debug_Printf("Send 01H error\r\n");
                }
                break;
            case 4:
                if (modbus_rtu_host_read_cmd_03H(hModbusRTU3, 0x08, 0x0000, 1) == 0)
                {
                    Debug_Printf("Send 01H error\r\n");
                }
                break;
            case 5:
                if (modbus_rtu_host_read_cmd_03H(hModbusRTU3, 0x06, 0x0000, 1) == 0)
                {
                    Debug_Printf("Send 01H error\r\n");
                }
                break;
            case MODBUSRTU3_DEV_NUM:
                modbusRTU3_devid = 0;
            default:
                break;
            }

            // 读取姿态（#50）传感器
            if (modbus_rtu_host_read_cmd_03H(hModbusRTU4, 0x50, 0x003D, 3) == 0)
            {
                Debug_Printf("Send 01H error\r\n");
            }
        }

        if (modbus_rtu_host_have_trans_event(hModbusRTU3))
        {
            switch (modbus_rtu_host_trans_addr(hModbusRTU3))
            {

            // 读取光照（#3）传感器,这个传感器只要485总线不空闲它就会处以一种状态，需要总线空闲100ms后才能读取。
            case 0x03:
            {
                Debug_Printf("收到寄存器值：%#02x，光照：%dW/m^2", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0]);
                Debug_Printf("\r\n");
                modbusRTU3_devid++;
                break;
            }

            // 读取风速（#1）传感器
            case 0x01:
            {
                Debug_Printf("收到寄存器值：%#02x，风速：%.2fm/s", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
                modbusRTU3_devid++;
                break;
            }

            // 读取风向（#2）传感器
            case 0x02:
            {
                Debug_Printf("收到寄存器值：%#02x，风向：%.2f°", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
                modbusRTU3_devid++;
                break;
            }

            // 读取锚索计（#4）传感器
            case 0x04:
            {
                Debug_Printf("收到寄存器值：%#02x，锚索计4：%.2fHz", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
                modbusRTU3_devid++;
                break;
            }

            // 读取锚索计（#8）传感器
            case 0x08:
            {
                Debug_Printf("收到寄存器值：%#02x，锚索计8：%.2fHz", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
                modbusRTU3_devid++;
                break;
            }

            // 读取锚索计（#6）传感器
            case 0x06:
            {
                Debug_Printf("收到寄存器值：%#02x，锚索计6：%.2fHz", hModbusRTU3->result.P[0], hModbusRTU3->result.P[0] / 10.0f);
                Debug_Printf("\r\n");
                modbusRTU3_devid++;
                break;
            }
            default:
                break;
            }
            modbus_rtu_host_clear_current_trans_event(hModbusRTU3);
        }

        if (modbus_rtu_host_have_trans_event(hModbusRTU4))
        {

            Debug_Printf("收到寄存器值：X %#04x,Y %#04x,Z %#04x,，姿态X：%.2f,Y:%.2f,Z:%.2f",
                         hModbusRTU4->result.P[0], hModbusRTU4->result.P[1], hModbusRTU4->result.P[2],
                         hModbusRTU4->result.P[0] / 32768.0f * 180,
                         hModbusRTU4->result.P[1] / 32768.0f * 180,
                         hModbusRTU4->result.P[2] / 32768.0f * 180);
            Debug_Printf("\r\n");
            modbus_rtu_host_clear_current_trans_event(hModbusRTU4);
        }
    }
}