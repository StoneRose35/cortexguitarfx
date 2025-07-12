
#include "pipicofx/014_ShimmerVerb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t ShimmerVerb::ShimmerVerb::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void ShimmerVerb::ShimmerVerb::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

void ShimmerVerb::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void ShimmerVerb::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void ShimmerVerb::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void ShimmerVerb::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void ShimmerVerb::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void ShimmerVerb::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void ShimmerVerb::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void ShimmerVerb::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}