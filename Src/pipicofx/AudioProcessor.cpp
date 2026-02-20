#include "pipicofx/AudioProcessor.hpp"
#include "memoryRegions.h"
using namespace PiPicoFX;
__QSPI_CODE
float AudioProcessor::processSample(float sampleIn)
{
    return sampleIn*0.0f;
}