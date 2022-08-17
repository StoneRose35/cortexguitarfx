#ifndef _FX_PROGRAM_H_
#define _FX_PROGRAM_H_
#include <stdint.h>
#include "audio/waveShaper.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/simpleChorus.h"
#include "audio/oversamplingWaveshaper.h"
#include "audio/gainstage.h"
#include "audio/bitcrusher.h"
#include "audio/delay.h"
#include "audio/compressor.h"

#define PARAMETER_NAME_MAXLEN 16
#define FXPROGRAM_NAME_MAXLEN 24
#define FXPROGRAM_MAX_PARAMETERS 8
#define N_FX_PROGRAMS 8


#define FXPROGRAM6_DELAY_TIME_LOWPASS_T 2
typedef int16_t(*processSampleCallback)(int16_t,void*);
typedef void(*paramChangeCallback)(uint16_t,void*);
typedef void(*setupCallback)(void*);
typedef void*(*getParameterValueFct)(void*);
typedef void(*getParameterDisplayFct)(void*,char*);

typedef struct {
    const char name[PARAMETER_NAME_MAXLEN];
    const uint8_t control; // 0-2: Potentiometers, 255: no control binding
    int16_t rawValue;
    int16_t increment;
    const getParameterValueFct getParameterValue; // returns the converted parameter value, data type depends on the implementation
    const getParameterDisplayFct getParameterDisplay; // returns the display value as a string of a Parameter
    paramChangeCallback setParameter; // sets the parameter in a meaningful way in the individual program
} FxProgramParameterType;

typedef struct {
    const char name[FXPROGRAM_NAME_MAXLEN];
    FxProgramParameterType parameters[FXPROGRAM_MAX_PARAMETERS];
    const processSampleCallback processSample;
    const setupCallback setup;
    const uint8_t nParameters;
    void * data;
} FxProgramType;


typedef struct {
    int16_t highpassCutoff;
    uint8_t nWaveshapers;
    int16_t highpass_out,highpass_old_out,highpass_old_in;
    WaveShaperDataType waveshaper1;
    FirFilterType filter3;
    SecondOrderIirFilterType filter1;
    DelayDataType * delay;
} FxProgram1DataType;

typedef struct {
    SimpleChorusType chorusData;
} FxProgram2DataType;

typedef struct {
    gainStageData gainStage;
    uint8_t cabSimType;
    uint8_t nWaveshapers;
    uint8_t waveshaperType;
    int16_t highpass_out,highpass_old_out,highpass_old_in;
    const char cabNames[6][24];
    const char waveShaperNames[4][24];
    FirFilterType hiwattFir;
    OversamplingWaveshaperDataType waveshaper1;
    SecondOrderIirFilterType hiwattIir1;
    SecondOrderIirFilterType hiwattIir2;
    SecondOrderIirFilterType hiwattIir3;
    
    FirFilterType frontmanFir;
    SecondOrderIirFilterType frontmanIir1;
    SecondOrderIirFilterType frontmanIir2;
    SecondOrderIirFilterType frontmanIir3;

    FirFilterType voxAC15Fir;
    SecondOrderIirFilterType voxAC15Iir1;
    SecondOrderIirFilterType voxAC15Iir2;
    SecondOrderIirFilterType voxAC15Iir3;
    

    //uint8_t updateLock;
} FxProgram4DataType;

typedef struct 
{
    BitCrusherDataType bitcrusher;
} FxProgram5DataType;

typedef struct 
{
    DelayDataType * delay;
} FxProgram6DataType;


typedef struct
{
    WaveShaperDataType waveshaper1;
    WaveShaperDataType waveshaper2;
    WaveShaperDataType waveshaper3;
    gainStageData gainStage;
    CompressorDataType compressor;
    int16_t highpass_out,highpass_old_out,highpass_old_in;
    const char cabNames[4][24];
    FirFilterType hiwattFir;
    FirFilterType frontmanFir;
    FirFilterType voxAC15Fir;
    SecondOrderIirFilterType cabF1;
    SecondOrderIirFilterType cabF2;
    SecondOrderIirFilterType cabF3;
    SecondOrderIirFilterType cabF4;    
    DelayDataType* delay;
    uint8_t cabSimType;
    
} FxProgram7DataType;


typedef struct 
{
    CompressorDataType compressor;
} FxProgram8DataType;


/* 
   ***************************************************************************
   ***************************************************************************
*/

FxProgramType fxProgram1;
FxProgramType fxProgram2;
FxProgramType fxProgram3;
FxProgramType fxProgram4;
FxProgramType fxProgram5;
FxProgramType fxProgram6;
FxProgramType fxProgram7;
FxProgramType fxProgram8;

FxProgramType * fxPrograms[N_FX_PROGRAMS];

#endif