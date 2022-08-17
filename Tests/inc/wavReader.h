#ifndef _WAV_READER_H_
#define _WAV_READER_H_
#include <stdint.h>
#include <stdio.h>

#define WAVREADER_FORMAT_ERROR 1
typedef struct {
    char chunkID[4]; // "RIFF"
    uint32_t ChunkSize; //Filesize in bytes minus 8
    char riffType[4]; // "WAVE"
} WavHeaderType;

typedef struct {
    char id[4]; // "fmt "
    uint32_t wFmtLength; // length of the format header
    uint16_t wFormatTag; // should be 1 (uncompressed)
    uint16_t wChannels; // 1: mono, 2: stereo
    uint32_t dwSamplesPerSec; // should be 48000
    uint32_t dwAvgBytesPerSec; // two (mono) or four (stereo) times the samples per sec for 16bit resolution
    uint16_t wBlockAlign; // frame size in bytes (4: stereo, 2: mono)
    uint16_t wBitsPerSample; // should be 16
} WavFormatType;

typedef struct {
    WavHeaderType wavHeader;
    WavFormatType wavFormat;
    uint32_t dataSize;
    int16_t * data;
    FILE * filePointer;
} WavFileType;

int openWavFile(char* filename,WavFileType*wavFile);
int createWavFile(char*filename,WavFileType*wavFile,uint32_t length);
void getNextSample(int16_t*sample,WavFileType*wavFile);
void writeNextSample(int16_t*sample,WavFileType*wavFile);
void writeWavFile(WavFileType*wavFile);
#endif