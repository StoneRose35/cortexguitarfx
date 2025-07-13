#ifndef _FX_PROGRAM_HPP_
#define _FX_PROGRAM_HPP_
#include "stdint.h"
#include "fxProgramParameter.hpp"
#include "AudioProcessor.hpp"
namespace PiPicoFX {
class FxProgram : public AudioProcessor
{
    public:
        FxProgram(uint8_t nParams,const char*);
        virtual ~FxProgram();
        void setup(void);
        virtual uint8_t addParameter(FxProgramParameter*p);
        uint8_t getParameterCount(void);
        virtual int16_t processSample(int16_t);
        FxProgramParameter* getParameter(uint8_t pos);
        const char * getName();
        void * data;
    private:
        uint8_t nParameters;
        uint8_t paramCnt;
        const char * programName;
        FxProgramParameter ** parameters;
};



};
#endif