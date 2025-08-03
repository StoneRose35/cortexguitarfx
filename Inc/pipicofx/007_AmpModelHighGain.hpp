
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/waveShaper.h"
#include "audio/delay.h"
#include "audio/reverb.h"
#include "audio/firFilter.h"
#include "audio/secondOrderIirFilter.h"
#include "picofxCore.hpp"
}

#define AMPMODEL_HIGHGAIN_HIGHCUT_VAL1 20000
#define AMPMODEL_HIGHGAIN_HIGHCUT_VAL2 31500
#define AMPMODEL_HIGHGAIN_HIGHCUT_DELTA (AMPMODEL_HIGHGAIN_HIGHCUT_VAL2-AMPMODEL_HIGHGAIN_HIGHCUT_VAL1)

namespace PiPicoFX {
    namespace AmpModelHighGain {
        class AmpModelHighGain : public FxProgram
        {
            public:
                AmpModelHighGain() : FxProgram(6,"Amp High Gain",DELAY_LINE_LENGTH<<1){
                    this->setup();
                };
                ~AmpModelHighGain();
                int16_t processSample(int16_t);

                int16_t highpassCutoff;
                uint8_t nWaveshapers;
                int16_t highpass_out,highpass_old_out,highpass_old_in;
                const char cabNames[9][24]={
                    "Custom             ",
                    "Custom IIR         ",
                    "Hiwatt M412     (F)", 
                    "Hiwatt M412     (I)",
                    "Fender Frontman (F)",
                    "Fender Frontman (I)",
                    "Vox AC15        (F)",
                    "Vox AC15        (I)",
                    "Off                " 
                };
                uint8_t modType;
                uint8_t cabSimType;
                MultiWaveShaperDataType waveshaper1;
                DelayDataType delay;
                ReverbType reverb;
                FirFilterType customCabFir={
                    .coefficients = {0x62c, 0x674, 0x7d6, 0xbc4, 0x1312, 0x1ea7, 0x2e33, 0x3b3a, 0x3a9e, 0x29bf, 0x15f5, 0x878, -0x67c, -0x1412, -0x17ed, -0x16c2, -0x13cc, -0xc2d, -0x2ee, 0x312, 0x5bf, 0x6eb, 0x5da, 0x487, 0x614, 0x771, 0x837, 0x784, 0x299, -0x372, -0x608, -0x42b, 0x2b, 0x44c, 0x599, 0x2f3, 0x43, 0x2, -0x1a5, -0x3c4, -0x2b3, 0xb7, 0x4ac, 0x823, 0xa3b, 0xa6a, 0x915, 0x74c, 0x69c, 0x6e5, 0x73e, 0x6cc, 0x4bf, 0x215, -0x52, -0x265, -0x21b, -0x2e, 0x222, 0x3cd, 0x50d, 0x5fa, 0x659, 0x61f}

                };
                SecondOrderIirFilterType customCabIir={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-15446, 5461},
                    .coeffB = {1599, 3199, 1599},
                };

                SecondOrderIirFilterType customIir1= {
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA={-28250, 13700},
                    .coeffB={229,459,229},
                };
                SecondOrderIirFilterType customIir2= {
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA={-29836,14408},
                    .coeffB={119,239,119}
                };
                SecondOrderIirFilterType customIir3={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA={-32700,16325},
                    .coeffB={2591,-5183,2591}
                };
                SecondOrderIirFilterType customIir4={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA={-31856,15497},
                    .coeffB={15934,-31868,15934}
                };

                FirFilterType hiwattFir= {
                    .coefficients= {-42, -42, -6, 317, 1371, 3572, 7006, 11147, 14461, 14434, 9110, 4, -7520, -9233, -8098, -6638, -4065, -601, 2424, 3345, 1940, 242, 302, 976, 809, 424, -56, -633, -650, -448, -767, -1168, -598, 952, 2050, 1294, -1130, -3011, -3906, -3785, -2258, -467, 841, 1033, 81, -771, -368, 398, 469, 505, 527, -281, -1373, -2394, -3074, -2888, -2175, -1570, -877, 258, 821, 194, -528, -772}
                };
                SecondOrderIirFilterType hiwattIir1={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-16557, 7532},
                    .coeffB =  {-319, -722, -382}
                };
                SecondOrderIirFilterType hiwattIir2={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {3428,3068},
                    .coeffB = {4426,5564, 871}
                };
                SecondOrderIirFilterType hiwattIir3={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {2860, 1804},
                    .coeffB = {-10189, -4775, -2987}
                };
                
                FirFilterType frontmanFir={
                    .coefficients = {-513, -1756, -3336, -5308, -7784, -9918, -9872, -6752, -2173, 1032, 1765, 1104, 18, 210, 2189, 4246, 4499, 2834, 138, -2553, -4127, -3345, 241, 4895, 7840, 7047, 3855, 1730, 986, -265, -2714, -4937, -5139, -3073, -59, 2420, 3183, 2875, 2107, -511, -4239, -5117, -1723, 3057, 5753, 5872, 5194, 4838, 5248, 6182, 4671, 1171, -1765, -2505, -1594, -619, -843, -1589, -749, 2533, 6338, 6855, 3017, -236}
                };
                SecondOrderIirFilterType frontmanIir1={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-16836, 5299},
                    .coeffB = {-148, -389, -436}
                };
                SecondOrderIirFilterType frontmanIir2={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-2990, -1681},
                    .coeffB = {-25230, -12670, 2490}
                };
                SecondOrderIirFilterType frontmanIir3={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-3343, -4287},
                    .coeffB = {-7552, 3249, 6865}
                };

                FirFilterType voxAC15Fir={
                    .coefficients = {-1110, -3127, -5900, -8409, -10180, -10897, -12238, -13579, -10051, -88, 7780, 6320, 468, -2894, -2063, 35, 2166, 2992, 2402, 1810, 2334, 2426, -195, -2013, 2505, 5798, 2919, 1492, 579, -925, -373, 1467, 2484, 2947, 3693, 3601, 1725, 241, 410, 1402, 2221, 2607, 2216, 776, -954, -1518, -677, 129, 1350, 3138, 130, -1561, -88, 646, 888, 1493, 1431, 811, 1202, 1587, 390, 548, 1266, 56}
                };
                SecondOrderIirFilterType voxAC15Iir1={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-10492, -1432},
                    .coeffB = {-465, -1525, -1652}
                };
                SecondOrderIirFilterType voxAC15Iir2={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-5755, 4143},
                    .coeffB = {-12044, 17013, 3125}
                };
                SecondOrderIirFilterType voxAC15Iir3={
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-1159, -1059},
                    .coeffB = {-13053, -3329, -548}
                };
                GainStageDataType presetVolume={
                    .gain=0xff,
                    .offset=0
                };
            private:
                void setup();
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(AmpModelHighGain* p) :FxProgramParameter(0,"Hi-Cut")
                {
                    rawValue = 4095;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(AmpModelHighGain* p) :FxProgramParameter(1,"Gain")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(AmpModelHighGain* p) :FxProgramParameter(2,"Mod Intensity")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(AmpModelHighGain* p) :FxProgramParameter(255,"Mod Type")
                {
                    rawValue = 0;
                    increment = 2048;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param5:  public FxProgramParameter
        {
            public:
                Param5(AmpModelHighGain* p) :FxProgramParameter(255,"Cab Type")
                {
                    rawValue = 0;
                    increment = 256;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param6:  public FxProgramParameter
        {
            public:
                Param6(AmpModelHighGain* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
    }
}
