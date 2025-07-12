
#include "pipicofx/007_AmpModelHighGain.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t AmpModelHighGain::AmpModelHighGain::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void AmpModelHighGain::AmpModelHighGain::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));

}

void AmpModelHighGain::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void AmpModelHighGain::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void AmpModelHighGain::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void AmpModelHighGain::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void AmpModelHighGain::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void AmpModelHighGain::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void AmpModelHighGain::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void AmpModelHighGain::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void AmpModelHighGain::Param5::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void AmpModelHighGain::Param5::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void AmpModelHighGain::Param6::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void AmpModelHighGain::Param6::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}