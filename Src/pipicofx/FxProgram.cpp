#include "pipicofx/FxProgram.hpp"
#include "memoryRegions.h"

using namespace PiPicoFX;

__QSPI_CODE
PiPicoFX::FxProgram::FxProgram(uint8_t nParams,const char * name,uint32_t memUseage)
{
    nParameters=nParams;
    paramCnt=0;
    programName = name;
    memoryUseage = memUseage;
    parameters = new FxProgramParameter*[nParameters];
}

__QSPI_CODE
PiPicoFX::FxProgram::~FxProgram()
{
    for (uint8_t c=0;c<this->nParameters;c++)
    {
        delete parameters[c];
    }
    delete parameters;
}

__QSPI_CODE
void FxProgram::setup(void){}

__QSPI_CODE
uint8_t FxProgram::addParameter(FxProgramParameter*p){

    if (paramCnt < nParameters)
    {
        parameters[paramCnt++] = p;
        return 0;
    }
    return 1;
}

__QSPI_CODE
uint8_t FxProgram::getParameterCount(void){
    return nParameters;
}

__QSPI_CODE
FxProgramParameter* FxProgram::getParameter(uint8_t pos){
    if (pos < nParameters)
    {
        return parameters[pos];
    }
    return nullptr;
}

__QSPI_CODE
float FxProgram::processSample(float sampleIn)
{
    if (this->getFreezeState() == FXP_FREEZE_STATE_FREEZING)
    {
        freezeCnt++;
        if (freezeCnt >= FXP_FREEZE_DURATION_IN_SAMPLES)
        {
            this->freezeCnt = FXP_FREEZE_DURATION_IN_SAMPLES -1;
            this->setFreezeState(FXP_FREEZE_STATE_FROZEN);
            this->onFreeze();
        }
    }
    else if (this->getFreezeState() == FXP_FREEZE_STATE_MELTING)
    {
        freezeCnt--;
        if (freezeCnt >= FXP_FREEZE_DURATION_IN_SAMPLES)
        {
            this->freezeCnt = 0;
            this->setFreezeState(FXP_FREEZE_STATE_MELTED);
        }
    }          
    return sampleIn;
}

__QSPI_CODE
uint32_t FxProgram::getDelayMemoryUseage(void)
{
    return this->memoryUseage;
}

__QSPI_CODE
const char * FxProgram::getName()
{
    return programName;
}

__QSPI_CODE
void FxProgram::setFreezable(uint8_t val)
{
    this->settingsState &= ~(1 << FXP_IS_FREEZABLE_POS);
    this->settingsState |= ((val &1) << FXP_IS_FREEZABLE_POS);
}

__QSPI_CODE
uint8_t FxProgram::isFreezable()
{
    return (settingsState & (1 << FXP_IS_FREEZABLE_POS)) >> FXP_IS_FREEZABLE_POS;
}

__QSPI_CODE
void FxProgram::freeze()
{
    if (this->isFreezable())
    {
        if (((this->settingsState & (0x3 << FXP_FREEZE_STATE_POS))) == FXP_FREEZE_STATE_MELTED || ((this->settingsState & (0x3 << FXP_FREEZE_STATE_POS))) == FXP_FREEZE_STATE_MELTING)
        {
            this->settingsState &= ~(0x3 << FXP_FREEZE_STATE_POS);
            this->settingsState |= FXP_FREEZE_STATE_FREEZING;
        }
    }
}

__QSPI_CODE
void FxProgram::unfreeze()
{
    if (this->isFreezable())
    {
        if (((this->settingsState & (0x3 << FXP_FREEZE_STATE_POS))) == FXP_FREEZE_STATE_FROZEN || ((this->settingsState & (0x3 << FXP_FREEZE_STATE_POS))) == FXP_FREEZE_STATE_FREEZING)
        {
            this->onMelt();
            this->settingsState &= ~(0x3 << FXP_FREEZE_STATE_POS);
            this->settingsState |= FXP_FREEZE_STATE_MELTING;
        }
    }
}

__QSPI_CODE
uint8_t FxProgram::isOn()
{
    return this->settingsState & (1 << FPX_STATE_POS);
}

__QSPI_CODE
void FxProgram::switchOn()
{
    this->settingsState |= (1 << FPX_STATE_POS);
    this->unfreeze();
}

__QSPI_CODE
uint8_t FxProgram::isFrozen()
{
    return ((this->settingsState >> FXP_FREEZE_STATE_POS) & 0x3) > 0;
}

__QSPI_CODE
uint8_t FxProgram::getFreezeState()
{
    return this->settingsState & (0x3 << FXP_FREEZE_STATE_POS);
}

__QSPI_CODE
void FxProgram::setFreezeState(uint8_t freezeState)
{
    this->settingsState &= ~(0x3 << FXP_FREEZE_STATE_POS);
    this->settingsState |= freezeState ;
}

__QSPI_CODE
uint8_t FxProgram::toggleOn()
{
    if (this->isFrozen())
    {
        this->switchOn();
        return 1;
    }
    if (this->isOn())
    {
        this->switchOff();
        return 0;
    }
    this->switchOn();
    return 1;
}

__QSPI_CODE
void FxProgram::switchOff()
{
    this->settingsState &= ~(1 << FPX_STATE_POS);
}


__QSPI_CODE
void FxProgram::onFreeze()
{

}

__QSPI_CODE
void FxProgram::onMelt()
{
    
}