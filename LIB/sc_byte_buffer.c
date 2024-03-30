#include "sc_byte_buffer.h"
void sc_byte_buffer_init(psc_byte_buffer p, byte *data, int capacity)
{
    p->data = data;
    p->capacity = capacity;
    p->size = 0;
}

int sc_byte_buffer_size(psc_byte_buffer p)
{
    return p->size;
}

int sc_byte_buffer_capacity(psc_byte_buffer p)
{
    return p->capacity;
}

/*
Return:
    success: 1
    fail: 0
*/
int sc_byte_buffer_push(psc_byte_buffer p, byte c)
{
    if (p->size < p->capacity)
    {
        p->data[p->size++] = c;
        return 1;
    }
    return 0;
}

/*
Return:
    successful pushed data length. This function just considering the input strings as a 
    uint8_t type array.
*/
int sc_byte_buffer_push_str(psc_byte_buffer p, byte *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (sc_byte_buffer_push(p, str[i]) == 0)
        {
            break;
        }
    }
    return i;
}

/*
Return: 实际存放的字节数。
*/
int sc_byte_buffer_push_data(psc_byte_buffer p, const byte *data, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if (sc_byte_buffer_push(p, data[i]) == 0)
        {
            break;
        }
    }
    return i;
}

/**
 * @brief set sc buffer size,but the size shoule less than it's capacity.
 *
 * @param p the pointer of sc_byte_buffer object.
 * @param size set size.
 * @return int fail:0:over capacity,success 1.
 */
int sc_byte_buffer_set_size(psc_byte_buffer p, int size)
{
    if (size > p->capacity)
    {
        return 0;
    }
    p->size = size;
    return 1;
}

void sc_byte_buffer_clear(psc_byte_buffer p)
{
    p->size = 0;
}

/**
 * @brief get data from sc_byte_buffer p by the index.
 * This method does not check for out-of-bounds errors.
 *
 * @param p
 * @param index
 * @return int the data of the sc_byte_buffer p at index.
 */
byte sc_byte_buffer_at(psc_byte_buffer p, uint32_t index)
{
    return p->data[index];
}