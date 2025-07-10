#ifndef _FXPROGRAM_AMPMODEL_HPP_
#define _FXPROGRAM_AMPMODEL_HPP_


#define FXPROGRAM1_HIGHCUT_VAL1 20000
#define FXPROGRAM1_HIGHCUT_VAL2 31500
#define FXPROGRAM1_HIGHCUT_DELTA (FXPROGRAM1_HIGHCUT_VAL2-FXPROGRAM1_HIGHCUT_VAL1)
#include "FxProgram.hpp"
extern "C" {
#include "audio/waveShaper.h"
#include "audio/firFilter.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/delay.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/gainstage.h"
#include "stringFunctions.h"


int16_t analogDelayFeedbackFunction(int16_t sampleIn,void*fbkFilterData,volatile uint32_t*audioStatePtr);

struct AmpModelData {
    int16_t highpassCutoff;
    uint8_t nWaveshapers;
    int16_t highpass_out,highpass_old_out,highpass_old_in;
    WaveShaperDataType waveshaper1;
    FirFilterType filter3;
    SecondOrderIirFilterType filter1;
    DelayDataType delay;
    FirstOrderIirType feedbackFilter;
    GainStageDataType presetVolume;
};


}
namespace PiPicoFX {
    namespace AmpModel {

    class Param1:  public FxProgramParameter
    {
        public:
            Param1(void * p) :FxProgramParameter(0,"Hi-Cut")
            {
                rawValue = 31500;
                increment = 1;
                pData=(AmpModelData*)p;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* chrbfr);
        private:
            AmpModelData * pData;
    };

    class Param2:  public FxProgramParameter
    {
        public:
            Param2(void * p) :FxProgramParameter(1,"Gain/Stages")
            {
                pData=(AmpModelData*)p;
                rawValue=0;
                increment=512;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* res);
        private:
            AmpModelData * pData;
    };

    class Param3:  public FxProgramParameter
    {
        public:
            Param3(void * p) :FxProgramParameter(2,"Delay Intensity")
            {
                pData=(AmpModelData*)p;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* res);
        private:
            AmpModelData * pData;
    };

    class Param4:  public FxProgramParameter
    {
        public:
            Param4(void * p) :FxProgramParameter(0xff,"Volume")
            {
                pData=(AmpModelData*)p;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* res);
        private:
            AmpModelData * pData;
    };

    class AmpModel : public FxProgram
    {
        public:
            AmpModel() : FxProgram(4,"Amp Model"){
                this->setup();
            };
            int16_t processSample(int16_t);
            void * data = &dataImpl;
        private:
            void setup(void);
            AmpModelData dataImpl={
                .highpassCutoff = 31000,
                .nWaveshapers = 1,
                .highpass_out=0,
                .highpass_old_out=0,
                .highpass_old_in=0,
                .filter3 = {
                .coefficients = {0x62c, 0x674, 0x7d6, 0xbc4, 0x1312, 0x1ea7, 0x2e33, 0x3b3a, 0x3a9e, 0x29bf, 0x15f5, 0x878, -0x67c, -0x1412, -0x17ed, -0x16c2, -0x13cc, -0xc2d, -0x2ee, 0x312, 0x5bf, 0x6eb, 0x5da, 0x487, 0x614, 0x771, 0x837, 0x784, 0x299, -0x372, -0x608, -0x42b, 0x2b, 0x44c, 0x599, 0x2f3, 0x43, 0x2, -0x1a5, -0x3c4, -0x2b3, 0xb7, 0x4ac, 0x823, 0xa3b, 0xa6a, 0x915, 0x74c, 0x69c, 0x6e5, 0x73e, 0x6cc, 0x4bf, 0x215, -0x52, -0x265, -0x21b, -0x2e, 0x222, 0x3cd, 0x50d, 0x5fa, 0x659, 0x61f}
                },
                /* butterworth lowpass @ 6000Hz */
                .filter1 = {
                    .x1=0,
                    .x2=0,
                    .y1=0,
                    .y2=0,
                    .acc=0,
                    .coeffA = {-15446, 5461},
                    .coeffB = {1599, 3199, 1599},
                },
                .delay={
                    .feedbackFunction=&analogDelayFeedbackFunction
                },
                .feedbackFilter={
                    .oldVal=0,
                    .oldXVal=0,
                    .alpha=14000
                },
                .presetVolume={
                    .gain=0xff,
                    .offset=0
                }
            };
    };
};
};

#endif