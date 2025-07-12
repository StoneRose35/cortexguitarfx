
#include "pipicofx/006_Compressor.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t Compressor::Compressor::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void Compressor::Compressor::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));

}

void Compressor::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Compressor::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Compressor::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Compressor::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Compressor::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Compressor::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Compressor::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Compressor::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Compressor::Param5::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Compressor::Param5::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Compressor::Param6::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Compressor::Param6::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}