
#include "pipicofx/004_MonsterCrusher.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t MonsterCrusher::MonsterCrusher::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void MonsterCrusher::MonsterCrusher::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));

}

void MonsterCrusher::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void MonsterCrusher::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void MonsterCrusher::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void MonsterCrusher::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}