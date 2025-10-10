#include "pipicofx/fxProgramParameter.hpp"

using namespace PiPicoFX;

FxProgramParameter::FxProgramParameter(const uint8_t ctrl, const char * pname): control(ctrl),parameterName(pname)
{

}


const char * FxProgramParameter::getParameterName()
{
    return parameterName;
}
uint8_t PiPicoFX::FxProgramParameter::getControl()
{
    return control;
}