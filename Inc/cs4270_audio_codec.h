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
#define CS4270_R4_DIG_LOOPBK_Pos (5)

#define CS4270_R6 (6)

void setupCS4270();

void cs4270PowerDown();

#endif