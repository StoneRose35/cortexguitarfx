#ifndef _FX_PROGRAM_HPP_
#define _FX_PROGRAM_HPP_
#include "stdint.h"
#include "fxProgramParameter.hpp"
#include "AudioProcessor.hpp"
#define FXP_IS_FREEZABLE_POS 1
#define FXP_STATE_OFF 0
#define FXP_STATE_ON 1
#define FPX_STATE_POS 0
#define FPX_IS_FREEZABLE (1 << FXP_IS_FREEZABLE_POS)
#define FXP_FREEZE_STATE_POS 2
#define FXP_FREEZE_STATE_MELTED (0 << FXP_FREEZE_STATE_POS)
#define FXP_FREEZE_STATE_FREEZING (1 << FXP_FREEZE_STATE_POS)
#define FXP_FREEZE_STATE_FROZEN (2 << FXP_FREEZE_STATE_POS)
#define FXP_FREEZE_STATE_MELTING (3 << FXP_FREEZE_STATE_POS)
#define FXP_FREEZE_DURATION_IN_SAMPLES 32
namespace PiPicoFX {
class FxProgram : public AudioProcessor
{
    public:
        FxProgram(uint8_t nParams,const char*,uint32_t memoryUseage,uint8_t idx);
        virtual ~FxProgram();
        void setup(uint8_t allocateMemory);
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
        uint8_t getFreezeState();
        void setFreezeState(uint8_t);
        virtual void onFreeze();
        virtual void onMelt();
        void switchOn();
        void switchOff();
        uint8_t isOn();
        uint8_t isFrozen();
        uint8_t toggleOn();
        uint8_t getIndex();
    private:
        uint8_t settingsState=0; //bit 0: state, 0: bypassed/off, 1: on, bit 1: freezable
                                 //  bit 2-3: freeze state, 0: melted, 1: freezing, 2: frozen, 3: melting 
        uint8_t nParameters;
        uint8_t paramCnt;
        const char * programName;
        uint32_t memoryUseage;
        FxProgramParameter ** parameters;
        uint8_t index;
    protected:
        uint32_t freezeCnt;
};



};
#endif