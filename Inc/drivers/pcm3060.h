#ifndef _PCM_3060_H_
#define _PCM_3060_H_

#include "stdint.h"

#define PCM3060_I2C_ADDRESS 70

#define PCM3060_R64 64
#define PCM3060_R65 65
#define PCM3060_R66 66
#define PCM3060_R67 67
#define PCM3060_R68 68
#define PCM3060_R69 69
#define PCM3060_R70 70
#define PCM3060_R71 71
#define PCM3060_R72 72
#define PCM3060_R73 73

#define PCM3060_R64_MRST 7
#define PCM3060_R64_SRST 6
#define PCM3060_R64_ADPSV 5
#define PCM3060_R64_DAPSV 4
#define PCM3060_R64_SE 0

#define PCM3060_R65_ATTENUATION_DAC_L 0 
#define PCM3060_R66_ATTENUATION_DAC_R 0
#define PCM3060_R67_CSEL2 7
#define PCM3060_R67_MS 4
#define PCM3060_R67_FMT 0
#define PCM3060_R68_OVER 6
#define PCM3060_R68_DREV 2
#define PCM3060_R68_MUT 0
#define PCM3060_R69_FLT 7
#define PCM3060_R69_DMF 5
#define PCM3060_R69_DMC 4
#define PCM3060_R69_ZREV 1
#define PCM3060_R69_AZRO 0
#define PCM3060_R70_ATTENUATION_ADC_L 0
#define PCM3060_R71_ATTENUATION_ADC_R 0
#define PCM3060_R72_CSEL1 7
#define PCM3060_R72_MS 4
#define PCM3060_R72_FMT 0
#define PCM3060_R73_ZCDD 4
#define PCM3060_R73_BYP 3
#define PCM3060_R73_DREV1 2
#define PCM3060_R73_MUT 0

#define PCM3060_CHANNEL_LEFT 1
#define PCM3060_CHANNEL_RIGHT 0
#define PCM3060_CHANNEL_BOTH 2

void pcm3060PowerDown();
void setupPCM3060();
uint8_t pcm3060GetInputState();
void pcm3060SetInputState(uint8_t channel,uint8_t val);
void pcm3060SetOutputVolume(uint8_t channel,uint8_t volume);
uint16_t pcm3060GetOutputVolume();

#endif