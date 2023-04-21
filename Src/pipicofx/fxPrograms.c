#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"



FxProgramType* fxPrograms[N_FX_PROGRAMS]={
    
    &fxProgram1, // amp model
    &fxProgram9, // amp model high gain
    &fxProgram14, // eq
    &fxProgram5, // monstercrusher
    &fxProgram8, // compressor
    &fxProgram2, // vibchorus
    &fxProgram11, // sine chorus
    &fxProgram6, // delay
    &fxProgram10, // reverb
    &fxProgram12, // allpass reverb
    &fxProgram13, // hadamard diffusor reverb
    &fxProgram3 // Off
    };
