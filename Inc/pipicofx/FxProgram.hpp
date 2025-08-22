#ifndef _FX_PROGRAM_HPP_
#define _FX_PROGRAM_HPP_
#include "stdint.h"
#include "fxProgramParameter.hpp"
#include "AudioProcessor.hpp"
namespace PiPicoFX {
class FxProgram : public AudioProcessor
{
    public:
        FxProgram(uint8_t nParams,const char*,uint32_t memoryUseage);
        virtual ~FxProgram();
        void setup(void);
        uint8_t addParameter(FxProgramParameter*p);
        int16_t processSample(int16_t);
        uint8_t getParameterCount(void);
        FxProgramParameter* getParameter(uint8_t pos);
        const char * getName();
        uint32_t getDelayMemoryUseage(); // returns the amount of delay memory required in bytes
        void * data;
    private:
        uint8_t nParameters;
        uint8_t paramCnt;
        const char * programName;
        uint32_t memoryUseage;
        FxProgramParameter ** parameters;
};



};
#endif