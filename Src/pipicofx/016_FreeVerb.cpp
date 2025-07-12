
#include "pipicofx/016_FreeVerb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t FreeVerb::FreeVerb::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void FreeVerb::FreeVerb::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

void FreeVerb::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void FreeVerb::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void FreeVerb::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void FreeVerb::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void FreeVerb::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void FreeVerb::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void FreeVerb::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void FreeVerb::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}