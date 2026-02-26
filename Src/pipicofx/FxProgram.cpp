#include "pipicofx/FxProgram.hpp"

using namespace PiPicoFX;

PiPicoFX::FxProgram::FxProgram(uint8_t nParams,const char * name,uint32_t memUseage)
{
    nParameters=nParams;
    paramCnt=0;
    programName = name;
    memoryUseage = memUseage;
    parameters = new FxProgramParameter*[nParameters];
}

PiPicoFX::FxProgram::~FxProgram()
{
    for (uint8_t c=0;c<this->nParameters;c++)
    {
        delete parameters[c];
    }
    delete parameters;
}

void FxProgram::setup(void){}
uint8_t FxProgram::addParameter(FxProgramParameter*p){

    if (paramCnt < nParameters)
    {
        parameters[paramCnt++] = p;
        return 0;
    }
    return 1;
}
uint8_t FxProgram::getParameterCount(void){
    return nParameters;
}
FxProgramParameter* FxProgram::getParameter(uint8_t pos){
    if (pos < nParameters)
    {
        return parameters[pos];
    }
    return nullptr;
}

int16_t FxProgram::processSample(int16_t sampleIn)
{
    return sampleIn;
}

uint32_t FxProgram::getDelayMemoryUseage(void)
{
    return this->memoryUseage;
}

const char * FxProgram::getName()
{
    return programName;
}

void FxProgram::setFreezable(uint8_t val)
{
    this->settingsState &= ~(1 << FXP_IS_FREEZABLE_POS);
    this->settingsState |= ((val &1) << FXP_IS_FREEZABLE_POS);
}

uint8_t FxProgram::isFreezable()
{
    return (settingsState & (1 << FXP_IS_FREEZABLE_POS)) >> FXP_IS_FREEZABLE_POS;
}

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

uint8_t FxProgram::isOn()
{
    return this->settingsState & (1 << FPX_STATE_POS);
}

void FxProgram::switchOn()
{
    this->settingsState |= (1 << FPX_STATE_POS);
    this->unfreeze();
}


uint8_t FxProgram::isFrozen()
{
    return ((this->settingsState >> FXP_FREEZE_STATE_POS) & 0x3) > 0;
}


uint8_t FxProgram::getFreezeState()
{
    return this->settingsState & (0x3 << FXP_FREEZE_STATE_POS);
}

void FxProgram::setFreezeState(uint8_t freezeState)
{
    this->settingsState &= ~(0x3 << FXP_FREEZE_STATE_POS);
    this->settingsState |= freezeState ;
}

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

void FxProgram::switchOff()
{
    this->settingsState &= ~(1 << FPX_STATE_POS);
}


void FxProgram::onFreeze()
{

}

void FxProgram::onMelt()
{
    
}