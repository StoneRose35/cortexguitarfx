#include "pipicofx/fxProgramParameter.hpp"
#include "memoryRegions.h"
using namespace PiPicoFX;

__QSPI_CODE
FxProgramParameter::FxProgramParameter(const uint8_t ctrl, const char * pname): control(ctrl),parameterName(pname)
{

}

__QSPI_CODE
const char * FxProgramParameter::getParameterName()
{
    return parameterName;
}
__QSPI_CODE
uint8_t PiPicoFX::FxProgramParameter::getControl()
{
    return control;
}