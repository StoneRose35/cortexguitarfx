#include "piRegulator.h"
#include <stdlib.h>
#include <stdio.h>
#include "stringFunctions.h"

static uint16_t tempHistory[16];
#define T_OUTSIDE_LAMP_OFF ((19 << 4))
#define T_OUTSIDE_LAMP_ON ((26<<4)+3) 
#define T_MAX ((32<<4)+8)
#define HEAT_COEFF 32
#define TIME_LAMP_ON (6*60*60+22*60)
#define TIME_LAMP_OFF (20*60*60+22*60)


void initSimulator()
{
    for(uint8_t c=0;c<16;c++)
    {
        tempHistory[c]=T_OUTSIDE_LAMP_OFF;
    }
}

uint16_t getSimulatedTemperature(TriopsControllerType * controller,uint32_t secondsOfDay)
{
    uint32_t interm_temp=0;
    uint32_t ntemp;

    if (secondsOfDay < TIME_LAMP_ON || secondsOfDay > TIME_LAMP_OFF)
    {
        interm_temp = tempHistory[0] +  (T_OUTSIDE_LAMP_OFF -  tempHistory[0])/HEAT_COEFF + (((controller->heaterValue*(T_MAX - T_OUTSIDE_LAMP_OFF)) >> 4)/1024);
    }
    else
    {
        interm_temp = tempHistory[0] +  (T_OUTSIDE_LAMP_ON -  tempHistory[0])/HEAT_COEFF + (((controller->heaterValue*(T_MAX - T_OUTSIDE_LAMP_OFF)) >> 4)/1024);
    }
    
    for (uint8_t c=15;c>0;c--)
    {
        tempHistory[c] = tempHistory[c-1];
    }
    tempHistory[0] = (uint16_t)interm_temp;

    return (uint16_t)interm_temp;
}

void main(int argc,char** argv)
{  
    TriopsControllerType tc;
    uint16_t tSim;
    FILE * fid;
    char nrbfr[16];
    char nrbfr2[16];
    tc.integralTempDeviation = 0;
    tc.tTarget = (25<<4) + 4;
    tc.tLower = (20<<4);
    tc.cIntegral = 50;
    tc.heaterValue = 0;
    initSimulator();
    fid = fopen("piregulator.txt","wt");
    for(uint32_t c=0;c<2*24*60*60;c+=30)
    {
        tSim = getSimulatedTemperature(&tc,c);
        getRegulatedHeaterValue(&tc,tSim);
        fixedPointUInt16ToChar(nrbfr,tSim,4);
        fixedPointInt16ToChar(nrbfr2,tc.integralTempDeviation,4);
        fprintf(fid,"%d, %s, %d, %s\r\n",c,nrbfr,tc.heaterValue,nrbfr2);
        printf("%d, %s, %d, %s\r\n",c,nrbfr,tc.heaterValue,nrbfr2);
    }
    fclose(fid);

}