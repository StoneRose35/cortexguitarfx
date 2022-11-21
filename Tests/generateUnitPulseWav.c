#include "inc/wavReader.h"

#define TOTLENGTH (48000*2)
int main(int argc,char**argv)
{
    WavFileType wavFile;
    createWavFile("audiosamples/unitPulse.wav",&wavFile,TOTLENGTH);
    for (uint32_t c=0;c<TOTLENGTH/2;c++)
    {
        if (c<2)
        {
            wavFile.data[c]=32767;
        }
        else
        {
            wavFile.data[c]=0;
        }
    }
    writeWavFile(&wavFile);
}