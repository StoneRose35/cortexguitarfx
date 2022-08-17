#ifndef _WM8731_H_
#define _WM8731_H_
#include "stdint.h"

#define WM8731_ADDRESS (50)

// register map
#define WM8731_R0 (0x0<<9)
#define LRIN_BOTH_LSB 8
#define LIN_MUTE_LSB 7
#define LIN_VOL 0  

#define WM8731_R1 (0x1<<9)
#define RLIN_BOTH_LSB 8
#define RIN_MUTE_LSB 7
#define RIN_VOL 0  

#define WM8731_R2 (0x2<<9)
#define LRHP_BOTH_LSB 8
#define LZCEN_LSB 7
#define LHPVOL_LSB 0 

#define WM8731_R3 (0x3<<9)
#define RLHP_BOTH_LSB 8
#define RZCEN_LSB 7
#define RHPVOL_LSB 0 

#define WM8731_R4 (0x4<<9)
#define SIDEATT_LSB 6
#define SIDETONE_LSB 5
#define DACSEL_LSB 4 
#define BYPASS_LSB 3
#define INSEL_LSB 2  
#define MUTE_MIC_LSB 1
#define MIC_BOOST_LSB 0

#define WM8731_R5 (0x5<<9)
#define HPOR_LSB 4
#define DACMU_LSB 3
#define DEEMPH_LSB 1
#define ADC_HPD_LSB 0

#define WM8731_R6 (0x6<<9)
#define PWR_OFF_LSB 7
#define CLK_OUTPD_LSB 6
#define OSCPD_LSB 5
#define OUTPD_LSB 4
#define DACPD_LSB 3
#define ADCPD_LSB 2
#define MICPD_LSB 1
#define LINEINPD_LSB 0

#define WM8731_R7 (0x7<<9)
#define BCLK_INV_LSB 7
#define MS_LSB 6
#define LRSWAP_LSB 5
#define LRP_LSB 4
#define IWL_LSB 2
#define FORMAT_LSB 0

#define WM8731_R8 (0x8<<9)
#define CLKO_DIV2_LSB 7
#define CLKI_DIV2_LSB 6
#define SR_LSB 2
#define BOSR_LSB 1
#define USB_NORM_LSB 0

#define WM8731_R9 (0x9<<9)
#define ACTIVE_LSB 0

#define WM8731_R15 (0xF<<9)

#define SAMPLEDEPTH_16BIT 0
#define SAMPLEDEPTH_20BIT 1
#define SAMPLEDEPTH_24BIT 2
#define SAMPLERATE_8KHZ 0
#define SAMPLERATE_32KHZ 1
#define SAMPLERATE_48KHZ 2

/**
 * @brief low-level register write function
 * 
 * @param data b15 - b9: register, b8 - b0: data
 */
void wm8731_write(uint16_t data);

/**
 * @brief sets the interface to the desired sample depth and sample rate
 * only both line inputs and headphone outputs are activated
 * mic is muted, any bypass is also muted
 * 
 * @param sampledepth 
 * @param samplerate 
 */
void setupWm8731(uint8_t sampledepth,uint8_t samplerate);




#endif