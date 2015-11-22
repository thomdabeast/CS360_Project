#include "type.h"


int tst_bit(char *buffer, int b)
{
    int byte = b / 8;
    int bit = b % 8;
    
    if (byte & (1 << bit))
        return 1;
    else
        return 0;
}

int get_bit(char *buffer, int index)
{
    int byte = index / 8;
    int bit = index % 8;
    
    if(buffer[byte] & (1 << bit))
        return 1;
    else
        return 0;
}

int set_bit(char *buf, int bit) // set bit_th bit in char buf[1024] to 1
{
    int i,j;
    i = bit / 8;
    j = bit % 8;
    buf[i] |= (1 << j);
    return 1;
}

int clear_bit(char *buf, int bit) // clear bit_th bit in char buf[1024] to 0
{
    int i, j;
    i = bit / 8;
    j = bit % 8;
    buf[i] &= ~(1 << j);
    return 1;
}