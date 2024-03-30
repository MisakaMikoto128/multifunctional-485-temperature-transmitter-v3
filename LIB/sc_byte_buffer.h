#ifndef SC_BYTE_BUFFER_H_
#define SC_BYTE_BUFFER_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define OVER_CAPACITY_ERROR -1
#ifndef byte
#define byte uint8_t
#endif
    /*
    sc_byte_buffer:
    主要用于字节数据缓冲区实现。sc表示static。
    */

    typedef struct _ByteBuffer
    {
        byte *data;   //缓冲区数据指针
        int size;     //当前缓冲数据长度
        int capacity; //缓冲容量
    } sc_byte_buffer, *psc_byte_buffer;

    void sc_byte_buffer_init(psc_byte_buffer p, byte *data, int capacity);
    int sc_byte_buffer_size(psc_byte_buffer p);
    int sc_byte_buffer_capacity(psc_byte_buffer p);
    int sc_byte_buffer_push(psc_byte_buffer p, byte c);
    int sc_byte_buffer_push_str(psc_byte_buffer p, byte *str);
    int sc_byte_buffer_push_data(psc_byte_buffer p, const byte *data, int len);
    int sc_byte_buffer_set_size(psc_byte_buffer p, int size);
    void sc_byte_buffer_clear(psc_byte_buffer p);
    byte sc_byte_buffer_at(psc_byte_buffer p,uint32_t index);
    #define sc_byte_buffer_data_ptr(p) ((const byte *)((p)->data))
#ifdef __cplusplus
}
#endif
#endif // SC_BYTE_BUFFER_H_