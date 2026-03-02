#pragma once

#define FXPROGRAM1_HIGHCUT_VAL1 0.61035f
#define FXPROGRAM1_HIGHCUT_VAL2 0.9613037f
#define FXPROGRAM1_HIGHCUT_DELTA (FXPROGRAM1_HIGHCUT_VAL2-FXPROGRAM1_HIGHCUT_VAL1)
#include "FxProgram.hpp"
extern "C" {
#include "audio/waveShaper.h"
#include "audio/firFilter.h"
#include "audio/secondOrderIirFilter.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/delay.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/gainstage.h"
#include "stringFunctions.h"
#include "picofxCore.hpp"
#include "memoryRegions.h"
float analogDelayFeedbackFunction(float sampleIn,void*fbkFilterData);


}
namespace PiPicoFX {
    namespace AmpModel {

    class AmpModel : public FxProgram
    {
        public:
            AmpModel() : FxProgram(4,"Amp Model",MAX_DELAY_SINGLEBUFFER<<2){
                this->setup();
            };

            ~AmpModel();
            float processSample(float);

            float highpassCutoff=31000.0f/32768.0f;
            uint8_t nWaveshapers=1;
            float highpass_out=0.0f,highpass_old_out=0.0f,highpass_old_in=0.0f;
            WaveShaperDataType waveshaper1;
            FirFilterType filter3 = {.coefficients = {0.016731f, 0.017496f, 0.021249f, 0.031896f, 0.051696f, 0.083098f, 0.125237f, 0.160554f, 0.158897f, 0.113163f, 0.059528f, 0.022957f, -0.017582f, -0.054411f, -0.064864f, -0.061696f, -0.053666f, -0.033009f, -0.007943f, 0.008333f, 0.015584f, 0.018759f, 0.015862f, 0.012281f, 0.016486f, 0.020179f, 0.022272f, 0.020375f, 0.007048f, -0.009343f, -0.016356f, -0.011307f, 0.000459f, 0.011650f, 0.015174f, 0.007995f, 0.000715f, 0.000025f, -0.004465f, -0.010208f, -0.007323f, 0.001944f, 0.012667f, 0.022059f, 0.027738f, 0.028239f, 0.024619f, 0.019782f, 0.017925f, 0.018693f, 0.019640f, 0.018431f, 0.012868f, 0.005646f, -0.000878f, -0.006494f, -0.005713f, -0.000488f, 0.005790f, 0.010304f, 0.013693f, 0.016206f, 0.017209f, 0.016596f, }
            ,.delayBuffer={},.filterLength=64,.delayPointer=0};
           
            SecondOrderIirFilterType filter1={            
                    .x1=0.0f,
                    .x2=0.0f,
                    .y1=0.0f,
                    .y2=0.0f,
                    .acc=0.0f,
                    .coeffA = {-0.94280904f, 0.33333333f},
                    .coeffB = {0.09763107f, 0.19526215f, 0.09763107f},};
            DelayDataType delay={.delayLine=0,
                .delayLinePtr=0,
                .delayInSamples=10,
                .feedback = 0.0f,
                .delayBufferLength = 0xff,
                .mix = 0.5f,
                .gainIn=1.0f,
                .feedbackFunction=&analogDelayFeedbackFunction,
                .feebackData = 0,
                .frozen=0};
            FirstOrderIirType feedbackFilter={                    
                    .oldVal=0.0f,
                    .oldXVal=0.0f,
                    .alpha=14000.0f/32768.0f,};
            GainStageDataType presetVolume = {
                .gain=1.0f,
                .offset=0.0f
            };
        private:
            void setup(void);
            void freeze() override;
            void unfreeze() override;
    };

    class Param1:  public FxProgramParameter
    {
        public:
            Param1(AmpModel* p) :FxProgramParameter(0,"Hi-Cut")
            {
                rawValue = 31500;
                increment = 1;
                pData=p;
                
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* chrbfr);
        private:
            AmpModel * pData;
    };

    class Param2:  public FxProgramParameter
    {
        public:
            Param2(AmpModel* p) :FxProgramParameter(1,"Gain/Stages")
            {
                pData=p;
                rawValue=0;
                increment=512;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* res);
        private:
            AmpModel * pData;
    };

    class Param3:  public FxProgramParameter
    {
        public:
            Param3(AmpModel* p) :FxProgramParameter(2,"Delay Intensity")
            {
                pData=p;
                rawValue=0;
                increment=1;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* res);
        private:
            AmpModel * pData;
    };

    class Param4:  public FxProgramParameter
    {
        public:
            Param4(AmpModel* p) :FxProgramParameter(0xff,"Volume")
            {
                pData=p;
                rawValue=0x3FF;
                increment=1;
            };
            void parameterCallback(uint16_t val);
            void parameterDisplay(char* res);
        private:
            AmpModel * pData;
    };

    
};
};
