#include "inc/wavReader.h"
#include "pipicofx/fxPrograms.h"
#include "romfunc.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "math.h"

#define SAMPLES_PER_PARAMETER 32

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

float fln(float a)
{
    return logf(a);
}

void eeprom24lc128WriteArray(uint32_t address,uint16_t size,uint8_t * data)
{

}

void eeprom24lc128ReadArray(uint32_t address,uint16_t size,uint8_t * data)
{

}

int main(int argc, char ** argv)
{
    const char * logfilename = "fxprogram_integrationtest.log";
    FILE * fid = fopen(logfilename,"wt");
    char displayBfr[24];
    int randnr, paramnr;
    uint16_t paramVal;
    uint64_t nVariations;
    for (uint32_t c=0;c<N_FX_PROGRAMS;c++)
    {
        fprintf(fid,"testing program %s\r\n\r\n",fxPrograms[c]->name);
        printf("testing program %s\r\n\r\n",fxPrograms[c]->name);
        fprintf(fid, "calling setup()\r\n");
        if (fxPrograms[c]->setup != 0)
        {
            printf("calling setup()\r\n");
            fxPrograms[c]->setup(fxPrograms[c]->data);
        }
        fprintf(fid,"systematic parameter check\r\n");
        printf("systematic parameter check\r\n");
        for (uint32_t np=0;np<fxPrograms[c]->nParameters;np++)
        {
            fprintf(fid,"\ttesting parameter %s\r\n",fxPrograms[c]->parameters[np].name);
            printf("\ttesting parameter %s\r\n",fxPrograms[c]->parameters[np].name);
            for(uint16_t v=0;v<4096;v++)
            {
                fxPrograms[c]->parameters[np].setParameter(v,fxPrograms[c]->data);
                fxPrograms[c]->parameters[np].getParameterDisplay(fxPrograms[c]->data,displayBfr);
                fprintf(fid,"\t\tset raw Value %u, Display: %s\r\n",v,displayBfr);
            }
        }
        fflush(fid);
        nVariations=1;
        for(uint8_t p=0;p<fxPrograms[c]->nParameters;p++)
        {
            nVariations *= SAMPLES_PER_PARAMETER;
        }
        if (nVariations > 1)
        {
            fprintf(fid,"shuffling through %lu variations\r\n",nVariations);
            printf("shuffling through %lu variations\r\n",nVariations);
            for (uint64_t cc=0;cc<nVariations;cc++)
            {
                randnr = rand();
                paramnr = randnr % fxPrograms[c]->nParameters;
                paramVal = (uint16_t)(randnr & 0xFFF);
                //fprintf(fid, "setting %s to %u\r\n", fxPrograms[c]->parameters[paramnr].name,paramVal);
                fxPrograms[c]->processSample((int16_t)(rand() & 0xFFFF),fxPrograms[c]->data);
            }
        }
    }
    fclose(fid);
}