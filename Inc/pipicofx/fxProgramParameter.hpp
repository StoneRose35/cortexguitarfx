#ifndef _FXPROGRAMPARAMETER_HPP_
#define _FXPROGRAMPARAMETER_HPP_
#include "stdint.h"
#include "memoryRegions.h"
namespace PiPicoFX {

    class FxProgram;
    class FxProgramParameter
{
    public:
        FxProgramParameter(const uint8_t ctrl,const char*);
        virtual ~FxProgramParameter()=default;
        virtual void parameterDisplay(char*)=0;
        virtual void parameterCallback(uint16_t val)=0;
        int16_t rawValue;
        int16_t increment;
        FxProgram * parent;
        const char * getParameterName();
        uint8_t getControl();
    private:
        const uint8_t control;
        const char * parameterName;

};
};
#endif