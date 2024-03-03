#ifndef _CS4270_AUDIO_CODEC_H_
#define _CS4270_AUDIO_CODEC_H_

#define CS4270_R2 (2)
#define CS4270_R2_PDN_Pos (0)

#define CS4270_R3 (3)
#define CS4270_R3_FM1_Pos (5)
#define CS4270_R3_FM0_Pos (4)

#define CS4270_R4 (4)
#define CS4270_R4_DAC_DIF1_Pos (4)
#define CS4270_R4_DAC_DIF0_Pos (3)
#define CS4270_R4_ADC_DIF0_Pos (0)

#define CS4270_R6 (6)
#define CS4270_R7 (7)
#define CS4270_R8 (8)

#define CS4270_I2C_ADDRESS 72
#define CS4270_CHANNEL_A 0 
#define CS4270_CHANNEL_B 1
#define CS4270_CHANNEL_BOTH 2

void setupCS4270();

void cs4270PowerDown();

void cs4270SetInputState(uint8_t channel,uint8_t val);
uint8_t cs4270GetInputState();
void cs4270SetOutputVolume(uint8_t channel,uint8_t volume);
uint16_t cs4270GetOutputVolume();
static uint8_t cs4270Write(uint16_t data);
static uint8_t cs4270Read(uint8_t reg);
#endif