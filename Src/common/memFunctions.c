/*
#include "stdlib.h"
#include "stdint.h"
void * memcpy ( void * destination, const void * source, size_t num )
{
    uint32_t c=0;
    if ((num&0x3) ==0) // copy word-wise if multiple of 4
    {
        for(c=0;c<(num >> 2);c++)
        {
            *(((uint32_t*)destination+c)) = *(((uint32_t*)source+c));
        }
    }
    else
    {
        for(c=0;c<num;c++)
        {
            *(((uint8_t*)destination+c)) = *(((uint8_t*)source+c));
        }
    }
    return destination;
}

void *memmove(void *str1, const void *str2, size_t n)
{
    uint8_t * tmp=malloc(n);
    uint32_t c;


    for(c=0;c<n;c++)
    {
        *(tmp+c) = *((char*)str2 + c); 
    }
    for(c=0;c<n;c++)
    {
        *((char*)str1+c) = *(tmp + c); 
    }

    free(tmp);
    return str1;
}

void * memset ( void * ptr, int value, size_t num )
{
    uint32_t c=0;
    for(c=0;c<num;c++)
    {
        *((uint8_t*)ptr + c) = (uint8_t)value;
    }
    return ptr;
}

*/