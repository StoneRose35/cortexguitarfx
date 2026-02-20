#ifndef _FX_PROGRAM_HPP_
#define _FX_PROGRAM_HPP_
#include "stdint.h"
#include "fxProgramParameter.hpp"
#include "AudioProcessor.hpp"
#define IS_FREEZABLE_POS 2
namespace PiPicoFX {
class FxProgram : public AudioProcessor
{
    public:
        FxProgram(uint8_t nParams,const char*,uint32_t memoryUseage);
        virtual ~FxProgram();
        void setup(void);
        uint8_t addParameter(FxProgramParameter*p);
        float processSample(float);
        uint8_t getParameterCount(void);
        FxProgramParameter* getParameter(uint8_t pos);
        const char * getName();
        uint32_t getDelayMemoryUseage(); // returns the amount of delay memory required in bytes
        void * data;
        void setFreezable(uint8_t);
        uint8_t isFreezable();
        virtual void freeze();
        virtual void unfreeze();
        void switchOn();
        void switchOff();
        uint8_t isOn();
        uint8_t isFrozen();
        uint8_t toggleOn();
    private:
        uint8_t settingsState=0; //bit 0-1: state, 0: bypassed/off, 1: on, 2: frozen, bit 2: freezable
        uint8_t nParameters;
        uint8_t paramCnt;
        const char * programName;
        uint32_t memoryUseage;
        FxProgramParameter ** parameters;
};



};
#endif