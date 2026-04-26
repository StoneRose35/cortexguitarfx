#include "pipicofx/MultiAudioProcessor.hpp"

using namespace PiPicoFX;

MultiAudioProcessor::MultiAudioProcessor()
{
    this->processors = new AudioProcessor*[MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH];
    for(uint8_t c=0;c<MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH;c++)
    {
        this->processors[c]=0;
    }
    this->routing = MULTI_AUDIO_PROCESSOR_ROUTING_3S;
    this->processorArrayLength=MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH;
}

uint8_t MultiAudioProcessor::addFxProgram(AudioProcessor*prg,uint8_t pos){
    if (pos < MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH)
    {
        this->processors[pos] = prg;
        return 0;
    }
    return 1;
}

AudioProcessor * MultiAudioProcessor::removeFxProgram(uint8_t pos)
{
    AudioProcessor * retval;
    if (pos < MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH && *(this->processors +pos) != 0)
    {
        retval = *(this->processors + pos);
        this->processors[pos]= 0;
        return retval;
    }
    return 0;
}

AudioProcessor * MultiAudioProcessor::getFxProgram(uint8_t pos)
{
    if (pos < MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH)
    {
        return this->processors[pos];
    }
    return nullptr;
}

void MultiAudioProcessor::swapPrograms(uint8_t posA,uint8_t posB)
{
    AudioProcessor * swapBfr;
    if (posA < MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH && posB < MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH)
    {
        swapBfr = *(this->processors + posA);
        *(this->processors + posA) = *(this->processors + posB);
        *(this->processors + posB) = swapBfr;
    }
}

uint8_t MultiAudioProcessor::getProgramListLength()
{
    return MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH;
}

MultiAudioProcessor::~MultiAudioProcessor()
{
    delete *this->processors;
}

uint8_t MultiAudioProcessor::getRouting()
{
    return this->routing;
}

uint8_t MultiAudioProcessor::setRouting(uint8_t r)
{
    if (r < 5)
    {
        this->routing = r;
        return 0;
    }
    return 1;
}

float MultiAudioProcessor::processSample(float s)
{
    float sampleOut=0.0f;;
    uint8_t processorsPresent = 0;
    switch (this->routing)
    {
        case MULTI_AUDIO_PROCESSOR_ROUTING_3S:
            sampleOut = s;
            if ((*(this->processors)) != 0)
            {
                sampleOut = (*(this->processors))->processSample(sampleOut);
            }
            if ((*(this->processors+1))!= 0)
            {
                sampleOut = (*(this->processors+1))->processSample(sampleOut);
            }
            if ((*(this->processors+2))!= 0)
            {
                sampleOut = (*(this->processors+2))->processSample(sampleOut);
            }
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_3P:
            sampleOut = 0.0f;
            if ((*(this->processors)) != 0)
            {
                sampleOut += (*(this->processors))->processSample(s);
                processorsPresent = 1;
            }
            if ((*(this->processors+1)) != 0)
            {
                sampleOut += (*(this->processors+1))->processSample(s);
                processorsPresent = 1;
            }
            if ((*(this->processors+2)) != 0)
            {
                sampleOut += (*(this->processors+2))->processSample(s);
                processorsPresent = 1;
            }
            if (!processorsPresent)
            {
                sampleOut = s;
            }
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_S_2P:
            sampleOut = s;
            if ((*(this->processors)) != 0)
            {
                sampleOut = (*(this->processors))->processSample(s);
            }
            if ((*(this->processors+1)) != 0)
            {
                sampleOut += (*(this->processors+1))->processSample(sampleOut);
            }            
            if ((*(this->processors+2)) != 0)
            {
                sampleOut += (*(this->processors+2))->processSample(sampleOut);
            }
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2P_S:
            sampleOut = 0.0f;
            if ((*(this->processors)) != 0)
            {
                sampleOut += (*(this->processors))->processSample(s);
                processorsPresent = 1;
            }
            if ((*(this->processors+1)) != 0)
            {
                sampleOut += (*(this->processors+1))->processSample(s);
                processorsPresent = 1;
            }
            if (!processorsPresent)
            {
                sampleOut = s;
            }
            if ((*(this->processors+2))!= 0)
            {
                sampleOut = (*(this->processors+2))->processSample(sampleOut);
            }
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2S_P:
            sampleOut = s;
            if ((*(this->processors)) != 0)
            {
                sampleOut = (*(this->processors))->processSample(sampleOut);
            }
            if ((*(this->processors+1))!= 0)
            {
                sampleOut = (*(this->processors+1))->processSample(sampleOut);
            }
            if ((*(this->processors+2))!= 0)
            {
                sampleOut += (*(this->processors+2))->processSample(s);
            }
            break;
    }
    return sampleOut;
}