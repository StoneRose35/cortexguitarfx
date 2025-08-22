#include  "pipicofx/MultiAudioProcessor.hpp"

using namespace PiPicoFX;

MultiAudioProcessor::MultiAudioProcessor()
{
    bufferLength = 4;
    length = 0;
    children = new AudioProcessor*[bufferLength];
}

MultiAudioProcessor::~MultiAudioProcessor()
{
    for (uint8_t c=0;c<length;c++)
    {
        delete children[c];
    }
    delete children;
}

int16_t MultiAudioProcessor::processSample(int16_t sample)
{
    int16_t sampleInterm=sample;
    if (mode==MODE_SERIAL)
    {
        for (uint8_t c=0;c<length;c++)
        {
            sampleInterm = children[c]->processSample(sampleInterm);
        }
    }
    else if (mode==MODE_PARALLEL)
    {
        sampleInterm = 0;
        for (uint8_t c=0;c<length;c++)
        {
            sampleInterm += children[c]->processSample(sample);
        }
    }
    return sampleInterm;
}

void MultiAudioProcessor::appendChild(AudioProcessor*el)
{
    if (length == bufferLength)
    {
        bufferLength += 4;
        AudioProcessor ** enlargedArray = new AudioProcessor*[bufferLength];
        for (uint8_t c=0;c<bufferLength-4;c++)
        {
            enlargedArray[c]=children[c];
        }
        children = enlargedArray;
    }
    children[length++] = el;
}

void MultiAudioProcessor::removeChild(uint8_t pos)
{
    if (pos < length)
    {
        for (uint8_t c = pos;c<length-1;c++)
        {
            children[c]= children[c+1];
        }
        length--;
    }
}

void MultiAudioProcessor::replaceChild(AudioProcessor* el,uint8_t pos)
{
    if (pos < length)
    {
        delete children[pos];
        children[pos] = el;
    }
}

AudioProcessor * MultiAudioProcessor::getChild(uint8_t pos)
{
    if (pos < length)
    {
        return children[pos];
    }
    return 0;
}

uint8_t MultiAudioProcessor::getLength()
{
    return length;
}