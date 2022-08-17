
#ifndef __ROM_FUNC
#define __ROM_FUNC

#include <stdint.h>
typedef struct bootRomInfo
{
    char magic1;
    char magic2;
    uint8_t magic3;
    uint8_t version;
} BootRomInfoType;

void * getRomData(char c1,char c2);

typedef void *(*rom_table_lookup_fn)(uint16_t *table, uint32_t code);
typedef void (*flash_range_erase_fn)(uint32_t addr,uint32_t count,uint32_t block_size,uint8_t block_cmd);
void flash_range_erase(uint32_t addr, uint32_t count, uint32_t block_size, uint8_t block_cmd);

typedef void (*flash_range_program_fn)(uint32_t addr, const uint8_t *data, uint32_t count);
void flash_range_program(uint32_t addr, const uint8_t *data, uint32_t count);

typedef  float (*f_proc_fct)(float a,float b);
typedef float(*f_proc_fct_conv)(uint32_t a);
typedef float(*f_proc_single)(float a);
typedef int32_t(*f_f2i_conv)(float a);
typedef float(*f_i2f_conv)(int32_t a);
void initFloatFunctions();

float __aeabi_fadd(float a,float b);
float __aeabi_fsub(float a,float b);
float __aeabi_frsub(float a,float b);
float __aeabi_fmul(float a,float b);
float __aeabi_fdiv(float a,float b);


float fsqrt(float a);
float fcos(float a);
float fsin(float a);
float ftan(float a);
float fexp(float a);
float fln(float a);
int32_t float2int(float a);
float int2float(int32_t a);

char * getCopyright();
void getBootRomInfo(BootRomInfoType** bootRomInfo);

#endif