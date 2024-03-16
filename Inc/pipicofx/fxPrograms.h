#ifndef _FX_PROGRAM_H_
#define _FX_PROGRAM_H_
#include <stdint.h>
#include "audio/waveShaper.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/simpleChorus.h"
#include "audio/sineChorus.h"
#include "audio/oversamplingWaveshaper.h"
#include "audio/gainstage.h"
#include "audio/bitcrusher.h"
#include "audio/delay.h"
#include "audio/compressor.h"
#include "audio/reverb.h"
#include "audio/reverb2.h"
#include "audio/reverb3.h"
#include "audio/threebandeq.h"
#include "audio/pitchshifter.h"
#include "pipicofx/picofxCore.h"


#define N_FX_PROGRAMS 13

#define FXPROGRAM6_DELAY_TIME_LOWPASS_T 2

typedef struct {
    float highpassCutoff;
    uint8_t nWaveshapers;
    float highpass_out,highpass_old_out,highpass_old_in;
    WaveShaperDataType waveshaper1;
    FirFilterType filter3;
    SecondOrderIirFilterType filter1;
    DelayDataType delay;
    FirstOrderIirType feedbackFilter;
} FxProgram1DataType;


typedef struct {
    float highpassCutoff;
    uint8_t nWaveshapers;
    float highpass_out,highpass_old_out,highpass_old_in;
    const char cabNames[9][24];
    const char waveShaperNames[4][24];
    uint8_t modType;
    uint8_t cabSimType;
    MultiWaveShaperDataType waveshaper1;
    DelayDataType delay;
    ReverbType reverb;
   
    FirFilterType customCabFir;
    SecondOrderIirFilterType customCabIir;

    SecondOrderIirFilterType customIir1;
    SecondOrderIirFilterType customIir2;
    SecondOrderIirFilterType customIir3;
    SecondOrderIirFilterType customIir4;

    FirFilterType hiwattFir;
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
} FxProgram9DataType;


typedef struct {
    SimpleChorusType chorusData;
} FxProgram2DataType;


typedef struct {
    GainStageDataType gainStage;
    uint8_t cabSimType;
    uint8_t nWaveshapers;
    uint8_t waveshaperType;
    float highpass_out,highpass_old_out,highpass_old_in;
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
    uint8_t resolution;
} FxProgram5DataType;

typedef struct 
{
    DelayDataType delay;
} FxProgram6DataType;


typedef struct
{
    WaveShaperDataType waveshaper1;
    WaveShaperDataType waveshaper2;
    WaveShaperDataType waveshaper3;
    GainStageDataType gainStage;
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
    uint8_t compressorType;
    CompressorDataType compressor;
    GainStageDataType makeupGain;
}  FxProgram8DataType;


typedef struct
{
    ReverbType reverb;
    float reverbTime;
} FxProgram10DataType;

typedef struct 
{
    SineChorusType sineChorus;
} FxProgram11DataType;


typedef struct 
{
    Reverb2Type reverb;
} FxProgram12DataType;

typedef struct 
{
    float mix;
    Reverb3Type reverb;
} FxProgram13DataType;

typedef struct
{
    ThreeBandEQType eq;
} FxProgram14DataType;

typedef struct 
{
    int16_t reverbTime;
    ThreeBandEQType eq;
    CompressorDataType comp;
    GainStageDataType postGain;
    ReverbType reverb;
} FxProgram15DataType;


typedef struct 
{
    PitchshifterDataType pitchShifter;
    int16_t mix;
} FxProgram16DataType;


extern FxProgramType fxProgram1;
extern FxProgramType fxProgram2;
extern FxProgramType fxProgram3;
extern FxProgramType fxProgram4;
extern FxProgramType fxProgram5;
extern FxProgramType fxProgram6;
extern FxProgramType fxProgram7;
extern FxProgramType fxProgram8;
extern FxProgramType fxProgram9;
extern FxProgramType fxProgram10;
extern FxProgramType fxProgram11;
extern FxProgramType fxProgram12;
extern FxProgramType fxProgram13;
extern FxProgramType fxProgram14;
extern FxProgramType fxProgram15;
extern FxProgramType fxProgram16;
extern FxProgramType* fxPrograms[N_FX_PROGRAMS];

#endif