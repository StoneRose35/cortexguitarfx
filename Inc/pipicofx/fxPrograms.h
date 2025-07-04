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
#include "audio/tremolo.h"
#include "pipicofx/picofxCore.h"


#define N_FX_PROGRAMS 15

#define FXPROGRAM6_DELAY_TIME_LOWPASS_T 2

#ifndef FLOAT_AUDIO
typedef struct {
    int16_t highpassCutoff;
    uint8_t nWaveshapers;
    int16_t highpass_out,highpass_old_out,highpass_old_in;
    WaveShaperDataType waveshaper1;
    FirFilterType filter3;
    SecondOrderIirFilterType filter1;
    DelayDataType delay;
    FirstOrderIirType feedbackFilter;
    GainStageDataType presetVolume;
} FxProgram1DataType;
#else
typedef struct {
    float highpassCutoff;
    uint8_t nWaveshapers;
    float highpass_out,highpass_old_out,highpass_old_in;
    WaveShaperDataType waveshaper1;
    FirFilterType filter3;
    SecondOrderIirFilterType filter1;
    DelayDataType * delay;
} FxProgram1DataType;
#endif


#ifndef FLOAT_AUDIO
typedef struct {
    int16_t highpassCutoff;
    uint8_t nWaveshapers;
    int16_t highpass_out,highpass_old_out,highpass_old_in;
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
    GainStageDataType presetVolume;
} FxProgram9DataType;
#else
typedef struct {
    float highpassCutoff;
    float highpass_out,highpass_old_out,highpass_old_in;
    MultiWaveShaperDataType waveshaper;
    FirFilterType filter3;
    SecondOrderIirFilterType filter1;
    DelayDataType * delay;
} FxProgram9DataType;
#endif

typedef struct {
    SimpleChorusType chorusData;
    GainStageDataType presetVolume;
} FxProgram2DataType;


typedef struct {
    GainStageDataType presetVolume;
} FxProgram3DataType;

#ifndef FLOAT_AUDIO
typedef struct {
    GainStageDataType gainStage;
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
#else
typedef struct {
    gainStageData gainStage;
    uint8_t cabSimType;
    uint8_t nWaveshapers;
    uint8_t waveshaperType;
    float highpass_out,highpass_old_out,highpass_old_in;
    const char cabNames[6][24];
    const char waveShaperNames[4][24];
    FirFilterType hiwattFir;
    MultiWaveShaperDataType waveshaper1;
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
#endif


typedef struct 
{
    BitCrusherDataType bitcrusher;
    uint8_t resolution;
    GainStageDataType presetVolume;
} FxProgram5DataType;

typedef struct 
{
    DelayDataType delay;
    GainStageDataType presetVolume;
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
    GainStageDataType presetVolume;
}  FxProgram8DataType;


typedef struct
{
    ReverbType reverb;
    int16_t reverbTime;
    GainStageDataType presetVolume;
} FxProgram10DataType;

typedef struct 
{
    SineChorusType sineChorus;
    GainStageDataType presetVolume;
} FxProgram11DataType;


typedef struct 
{
    Reverb2Type reverb;
    GainStageDataType presetVolume;
} FxProgram12DataType;

typedef struct 
{
    int16_t mix;
    Reverb3Type reverb;
    GainStageDataType presetVolume;
} FxProgram13DataType;

typedef struct
{
    ThreeBandEQType eq;
    GainStageDataType presetVolume;
} FxProgram14DataType;

typedef struct 
{
    int16_t reverbTime;
    ThreeBandEQType eq;
    CompressorDataType comp;
    GainStageDataType postGain;
    ReverbType reverb;
    GainStageDataType presetVolume;
} FxProgram15DataType;


typedef struct 
{
    Pitchshifter2DataType pitchShifter;
    int16_t mix;
    GainStageDataType presetVolume;
} FxProgram16DataType;


typedef struct 
{

    Pitchshifter2DataType pitchShifter;
    FirstOrderIirType glitterTamer;
    DelayDataType delays[4];
    AllpassType allpasses[2];
    int16_t oldVal;
    int16_t feedback;
    int16_t mix;
    GainStageDataType presetVolume;
} FxProgram17DataType;

typedef struct 
{
    TremoloType tremolo;
    GainStageDataType presetVolume;
} FxProgram18DataType;



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
extern FxProgramType fxProgram17;
extern FxProgramType fxProgram18;
extern FxProgramType* fxPrograms[N_FX_PROGRAMS];

#endif