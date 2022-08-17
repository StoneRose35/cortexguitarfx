#include "./../inc/wavReader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int wavReader_main(int argc,char** argv)
{
    WavFileType inFile;
    WavFileType outFile;
    uint16_t sample[2];
    
    openWavFile("./audiosamples/guit_riff_16bit.wav",&inFile);
    createWavFile("testout.wav",&outFile,23);

}

int openWavFile(char* filename,WavFileType*wavFile)
{
    char chunkID[5];
    uint32_t junkSize;
    wavFile->filePointer = fopen(filename,"rb");
    fread(&wavFile->wavHeader,sizeof(WavHeaderType),1,wavFile->filePointer);
    fread(&chunkID,4,1,wavFile->filePointer);
    chunkID[4]=0;
    if (strcmp(chunkID,"JUNK")==0)
    {
        fread(&junkSize,4,1,wavFile->filePointer);
        
        if ((junkSize & 1)!= 0)
        {
            fseek(wavFile->filePointer,junkSize+1,SEEK_CUR);
        }
        else
        {
            fseek(wavFile->filePointer,junkSize,SEEK_CUR);
        }
    }
    else
    {
        fseek(wavFile->filePointer,-4,SEEK_CUR);
    }
    fread(&wavFile->wavFormat,sizeof(WavFormatType),1,wavFile->filePointer);
    // skip the "data" tag
    fseek(wavFile->filePointer,4,SEEK_CUR);
    // read the data size
    fread(&wavFile->dataSize,4,1,wavFile->filePointer);
    if (strncmp(wavFile->wavHeader.chunkID,"RIFF",4)!=0)
    {
        return WAVREADER_FORMAT_ERROR;
    }
    if (strncmp(wavFile->wavHeader.riffType,"WAVE",4)!=0)
    {
        return WAVREADER_FORMAT_ERROR;
    }    
    if (wavFile->wavFormat.wFormatTag != 1)
    {
        return WAVREADER_FORMAT_ERROR;
    }
    if (wavFile->wavFormat.wChannels < 1 || wavFile->wavFormat.wChannels > 2)
    {
        return WAVREADER_FORMAT_ERROR;
    }    
    if (strncmp(wavFile->wavFormat.id,"fmt ",4) !=0)
    {
        return WAVREADER_FORMAT_ERROR;
    } 
    if (wavFile->wavFormat.wBitsPerSample != 16)
    {
        return WAVREADER_FORMAT_ERROR;
    }
    wavFile->data =  malloc(wavFile->dataSize);
    uint32_t c=0;
    fread(wavFile->data,wavFile->dataSize,1,wavFile->filePointer);
    return 0;
}

int createWavFile(char*filename,WavFileType*wavFile,uint32_t length)
{
    const uint32_t zeroSize=0;
    wavFile->filePointer = fopen(filename,"wb");
    wavFile->wavHeader.chunkID[0] = 'R';
    wavFile->wavHeader.chunkID[1] = 'I';
    wavFile->wavHeader.chunkID[2] = 'F';
    wavFile->wavHeader.chunkID[3] = 'F';
    wavFile->wavHeader.riffType[0] = 'W';
    wavFile->wavHeader.riffType[1] = 'A';
    wavFile->wavHeader.riffType[2] = 'V';
    wavFile->wavHeader.riffType[3] = 'E';

    wavFile->wavFormat.dwAvgBytesPerSec = 2*48000;
    wavFile->wavFormat.dwSamplesPerSec=48000;
    wavFile->wavFormat.id[0] = 'f';
    wavFile->wavFormat.id[1] = 'm';    
    wavFile->wavFormat.id[2] = 't';
    wavFile->wavFormat.id[3] = ' ';
    wavFile->wavFormat.wBitsPerSample = 16;
    wavFile->wavFormat.wBlockAlign = 2;
    wavFile->wavFormat.wChannels = 1;
    wavFile->wavFormat.wFormatTag = 1;
    wavFile->wavFormat.wFmtLength = 16;

    fwrite(&wavFile->wavHeader,sizeof(WavHeaderType),1,wavFile->filePointer);
    fwrite(&wavFile->wavFormat,sizeof(WavFormatType),1,wavFile->filePointer);
    const char dataId[4]={'d','a','t','a'};
    fwrite(dataId,4,1,wavFile->filePointer);
    fwrite(&length,4,1,wavFile->filePointer); 
    wavFile->dataSize=length;
    wavFile->data = malloc(length);
}

void getNextSample(int16_t*sample,WavFileType*wavFile)
{
    if(wavFile->wavFormat.wChannels == 2)
    {
        fread(sample,2,2,wavFile->filePointer);
    }
    else
    {
        fread(sample,2,1,wavFile->filePointer);
    }
}


void writeWavFile(WavFileType*wavFile)
{
    fwrite(wavFile->data,2,wavFile->dataSize>>1,wavFile->filePointer);
}
void writeNextSample(int16_t*sample,WavFileType*wavFile)
{
    long currentPos;
    uint32_t chunkSize;
    if(wavFile->wavFormat.wChannels == 2)
    {
        wavFile->dataSize += 4;
        fwrite(sample,2,2,wavFile->filePointer);
    }
    else
    {
        wavFile->dataSize +=2;
        fwrite(sample,2,1,wavFile->filePointer);
    }
    currentPos = ftell(wavFile->filePointer);
    rewind(wavFile->filePointer);
    chunkSize = wavFile->dataSize + sizeof(WavHeaderType) + sizeof(WavFormatType);
    wavFile->wavHeader.ChunkSize = chunkSize;
    fwrite(&wavFile->wavHeader,sizeof(WavHeaderType),1,wavFile->filePointer);
    fseek(wavFile->filePointer,sizeof(WavFormatType)+4,SEEK_CUR);
    fwrite(&wavFile->dataSize,4,1,wavFile->filePointer);
    fseek(wavFile->filePointer,currentPos,SEEK_SET);
}