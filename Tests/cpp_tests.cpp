#include <iostream>
#include "stdint.h"
#include "math.h"

#include "pipicofx/FXProgram_AmpModel.hpp"
#include "pipicofx/FxProgram.hpp"


extern "C" 
{
    float int2float(int32_t a)
    {
        return (float)a;
    }

    int32_t float2int(float a)
    {
        return (int32_t)a;
    }

    float fln(float a)
    {
        return logf(a);
    }
}


int main(int argc,char ** argv)
{
    char  displayBfr[32];
    int16_t sampleIn, sampleOut;
    uint16_t paramValues[]={0, 1024, 2048,3072,4095,1023};
    PiPicoFX::FxProgram  * prog1 = new PiPicoFX::AmpModel::AmpModel();
    std::cout << "We've got FX Program \"" << prog1->getName() <<"\"\n";
    std::cout << "It has " << prog1->getParameterCount() << " editable Parameters\n";
    for (uint8_t c=0;c< prog1->getParameterCount();c++)
    {
        prog1->getParameter(c)->parameterDisplay(displayBfr);
        std::cout << "\t Parameter " << c+1 << ": " << prog1->getParameter(c)->getParameterName() << ", current value: " << displayBfr << "\n";
    }
    std::cout << "Setting some Parameter Values\n";

    for (uint8_t c=0;c< prog1->getParameterCount();c++)
    {
        for (uint16_t q=0;q<(sizeof(paramValues)>>1);q++)
        {
            prog1->getParameter(c)->parameterCallback(q);
            prog1->getParameter(c)->parameterDisplay(displayBfr);
            std::cout << "Setting " << prog1->getParameter(c)->getParameterName() << " to " << q << " (raw), yields " << displayBfr << "\n";
        }
    }
    std::cout << "Processing a few Samples\n";
    for (uint16_t c=0;c<512;c++)
    {
        sampleIn = ((c-256)<<7);
        sampleOut = prog1->processSample(sampleIn);
        std::cout << "In: " << sampleIn << ", Out: " << sampleOut << "\n"; 
    } 

}
