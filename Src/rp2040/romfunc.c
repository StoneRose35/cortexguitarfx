#include <stdint.h>
#include "romfunc.h"

volatile f_proc_fct _fadd=0;
volatile f_proc_fct _fsub=0;
volatile f_proc_fct _fmul=0;
volatile f_proc_fct _fdiv=0;
volatile f_proc_single _fsqrt=0;
volatile f_proc_single _fcos=0;
volatile f_proc_single _fsin=0;
volatile f_proc_single _ftan=0;
volatile f_proc_single _fexp=0;
volatile f_proc_single _fln=0;
volatile f_proc_fct_conv uint32ToFloat=0;
volatile f_f2i_conv _f2i=0;
volatile f_i2f_conv _i2f=0;

void initFloatFunctions()
{
    void * floatFunctionsTable = getRomData('S','F');
    _fadd = (f_proc_fct)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x0));
    _fsub = (f_proc_fct)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x4));
    _fmul = (f_proc_fct)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x8));
    _fdiv = (f_proc_fct)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0xc));
    _fsqrt = (f_proc_single)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x18));
    _fcos = (f_proc_single)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x3c));
    _fsin = (f_proc_single)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x40));
    _ftan = (f_proc_single)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x44));
    _fexp = (f_proc_single)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x4c));
    _fln = (f_proc_single)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x50));
    _i2f = (f_i2f_conv)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x2c));
    _f2i = (f_f2i_conv)(uint32_t)(*(uint16_t*)(floatFunctionsTable+0x1c));
}


void * getRomFunction(char c1,char c2)
{
    void * fct;
    uint32_t code = c1 | (c2 << 8);
    //multicast (uint32_t -> void pointer -> function pointer/uint16_t pointer) here only to avoid compiler warnings
    rom_table_lookup_fn rom_table_lookup = (rom_table_lookup_fn)(void*)(uint32_t)(*((uint16_t*)0x18));
    fct = rom_table_lookup((uint16_t*)(void*)(uint32_t)*((uint16_t*)0x14),code);
    return fct;
}



void * getRomData(char c1,char c2)
{
    void * fct;
    uint32_t code = c1 | (c2 << 8);
    //multicast (uint32_t -> void pointer -> function pointer/uint16_t pointer) here only to avoid compiler warnings
    rom_table_lookup_fn rom_table_lookup = (rom_table_lookup_fn)(void*)(uint32_t)(*((uint16_t*)0x18));
    fct = rom_table_lookup((uint16_t*)(void*)(uint32_t)*((uint16_t*)0x16),code);
    return fct;
}


void flash_range_erase(uint32_t addr, uint32_t count, uint32_t block_size, uint8_t block_cmd)
{
    flash_range_erase_fn flash_range_erase_ptr = getRomFunction('R','E');
    flash_range_erase_ptr(addr,count,block_size,block_cmd);
}

void flash_range_program(uint32_t addr,const uint8_t * data, uint32_t cnt)
{
    flash_range_program_fn flash_range_program_ptr = getRomFunction('R','P');
    flash_range_program_ptr(addr,data,cnt);
}

/**
 * @brief a+b
 * 
 * @param a 
 * @param b 
 * @return float 
 */
float __aeabi_fadd(float a,float b)
{
    if (_fadd == 0)
    {
        initFloatFunctions();
    }
    return _fadd(a,b);
}

/**
 * @brief a-b
 * 
 * @param a 
 * @param b 
 * @return float 
 */
float __aeabi_fsub(float a,float b)
{
    if (_fsub == 0)
    {
        initFloatFunctions();
    }
    return _fsub(a,b);
}


float __aeabi_frsub(float a,float b)
{
    if (_fsub == 0)
    {
        initFloatFunctions();
    }
    return _fsub(b,a);
}

/**
 * @brief a*b
 * 
 * @param a 
 * @param b 
 * @return float 
 */
float __aeabi_fmul(float a,float b)
{
    if (_fmul == 0)
    {
        initFloatFunctions();
    }
    return _fmul(a,b);
}

/**
 * @brief a/b
 * 
 * @param a 
 * @param b 
 * @return float 
 */
float __aeabi_fdiv(float a,float b)
{
    if (_fdiv == 0)
    {
        initFloatFunctions();
    }
    return _fdiv(a,b);
}

float fsqrt(float a)
{
    if (_fsqrt == 0)
    {
        initFloatFunctions();
    }
    return _fsqrt(a);
}

float fcos(float a)
{
    if (_fcos == 0)
    {
        initFloatFunctions();
    }
    return _fcos(a);
}

float fsin(float a)
{
    if (_fsin == 0)
    {
        initFloatFunctions();
    }
    return _fsin(a);
}
float ftan(float a)
{
    if (_ftan == 0)
    {
        initFloatFunctions();
    }
    return _ftan(a);
}

float fexp(float a)
{
    if (_fexp == 0)
    {
        initFloatFunctions();
    }
    return _fexp(a);
}

float fln(float a)
{
    if (_fln == 0)
    {
        initFloatFunctions();
    }
    return _fln(a);
}

int32_t float2int(float a)
{
    if (_f2i == 0)
    {
        initFloatFunctions();
    }
    return _f2i(a);
}

float int2float(int32_t a)
{
    if (_i2f == 0)
    {
        initFloatFunctions();
    }
    return _i2f(a);
}

float fatan2(float a, float b)
{
    return 0.0;
}


char * getCopyright()
{
    void * copyrightPtr = getRomData('C','R');
    return (char*)copyrightPtr;
}

void getBootRomInfo(BootRomInfoType** bootRomInfo)
{
    *bootRomInfo = (BootRomInfoType*)0x10;
}