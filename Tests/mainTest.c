#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "consoleHandler.h"
#include "taskManager.h"
#include "taskManagerUtils.h"
#include "bufferedInputHandler.h"
#include "system.h"
#include "stringFunctions.h"
#include "demoColorInterpolator.h"
#include "colorInterpolator.h"
#include "interpolators.h"
#include "flash.h"
#include "memoryAccess.h"





TaskType interpolatorsArray[N_LAMPS];
TasksType interpolators;

RGBStream lampsarray[N_LAMPS];
RGBStream * lamps=lampsarray;

BufferedInputType bufferedInputType;
BufferedInput  bufferedInput=&bufferedInputType;

void consoleHandlerHistoryCheck()
{
	ConsoleType c1;
	printf("******** starting consoleHandlerHistoryCheck\n");
	initConsole(&c1);
	bufferedInput->console = &c1;

	onCharacterReception(bufferedInput,(uint8_t)'A');
	onCharacterReception(bufferedInput,(uint8_t)'Q');
	onCharacterReception(bufferedInput,(uint8_t)'U');
	onCharacterReception(bufferedInput,(uint8_t)'A');
	onCharacterReception(bufferedInput,(uint8_t)'(');
	onCharacterReception(bufferedInput,(uint8_t)'0');
	onCharacterReception(bufferedInput,(uint8_t)')');
	onCharacterReception(bufferedInput,(uint8_t)'\r');

	// simulate up arrow
	onCharacterReception(bufferedInput,(uint8_t)'\e');
	onCharacterReception(bufferedInput,(uint8_t)'[');
	onCharacterReception(bufferedInput,(uint8_t)'A');

	// delete two characters
	onCharacterReception(bufferedInput,127);
	onCharacterReception(bufferedInput,127);

	// write a "1)", text should now be AQUA(1)
	onCharacterReception(bufferedInput,(uint8_t)'1');
	onCharacterReception(bufferedInput,(uint8_t)')');


	onCharacterReception(bufferedInput,(uint8_t)'\r');

	if (!(lamps->rgb.r == 0 && lamps->rgb.g == 210 && lamps->rgb.b == 140))
	{
		printf("test failed, color of first lamp is wrong\n");
	}

	if (!((lamps+1)->rgb.r == 0 && (lamps+1)->rgb.g == 210 && (lamps+1)->rgb.b == 140))
	{
		printf("test failed, color of second lamp is wrong\n");
	}

	printf("******** ending consoleHandlerHistoryCheck\n");
}

void testExpandRange()
{
	uint8_t * resultdata;
	uint8_t len;

	printf("******** starting testExpandRange\n");

	char rangedef[16]="3-7";

	len=expandRange(rangedef,&resultdata);
	if (resultdata[0] - 3 != 0)
	{
		printf("test failed\n");
	}
	if (resultdata[4] - 7 != 0)
	{
		printf("test failed\n");
	}
	if(len!=5)
	{
		printf("test failed, length should be 5\n");
	}
	printf("******** ending testExpandRange\n");
}

void testExpandDescription()
{
	printf("******** starting testExpandDescription\n");
	char descr[16]="4";
	uint8_t len;
	uint8_t lampnrarray[N_LAMPS];
	len = expandLampDescription(descr,lampnrarray);
	if(len != 1)
	{
		printf("test failed, length should be 1\n");
	}

	char descr2[] = "4,2,2,7";
	len = expandLampDescription(descr2,lampnrarray);
	if(len != 3)
	{
		printf("test failed, length should be 3\n");
	}
	if (lampnrarray[0]!= 2)
	{
		printf("test failed, first element should be 2\n");
	}

	char descr3[] = "4,2,7, 3 - 6";
	len = expandLampDescription(descr3,lampnrarray);
	if(len != 6)
	{
		printf("test failed, length should be 6\n");
	}
	printf("******** ending testExpandDescription\n");
}


void testRgbCommand()
{
	printf("******** starting testRgcCommand\n");
	char command[32] = "RGB(23,34,45,1)";
	handleCommand(command,0);
	if (lamps[1].rgb.r != 23 || lamps[1].rgb.g != 34 || lamps[1].rgb.b != 45)
	{
		printf("test failed: color set for lamp 1 is incorrect\n");
	}
	printf("******** ending testRgcCommand\n");
}

void testConvertInts()
{
	printf("******** starting testConvertInts\n");
	uint8_t v1;
	int16_t v2;
	v1 = toUInt8("56");
	if (v1 != 56)
	{
		printf("test failed: should return 56\n");
	}

	v2 = toInt16("-134");
	if (v2 != -134)
	{
		printf("test failed: should return -134\n");
	}

	v2 = toInt16("13443");
	if (v2 != 13443)
	{
		printf("test failed: should return 13443\n");
	}
	printf("******** ending testConvertInts\n");
}

void testPercentToChar()
{
	printf("******** starting testPercentToChar\n");
	char nrbfr[8];
	float a=0.784528;

	toPercentChar(a,nrbfr);
	if (!(nrbfr[0] == '7' &&
			nrbfr[1] == '8' &&
			nrbfr[2] == '.' &&
			nrbfr[3] == '4' &&
			nrbfr[4] == '5' &&
			nrbfr[5] == '2' ))
	{
		printf("test failed, returned string is not 78.452\n");
	}
	printf("******** ending testPercentToChar\n");
}

void testFillWithLeadingZeros()
{
	printf("******** starting testFillWithLeadingZeros\n");
	char nrbfr[8];
	nrbfr[0]=0x30;
	nrbfr[1]=0;

	fillWithLeadingZeros(4,nrbfr);
	if ((uint8_t)nrbfr[0] != 0x30)
	{
		printf("testFillWithLeadingZeros failed, pos 1 is not 0");
	}
	if ((uint8_t)nrbfr[1] != 0x30)
	{
		printf("testFillWithLeadingZeros failed, pos 2 is not 0");
	}
	if ((uint8_t)nrbfr[2] != 0x30)
	{
		printf("testFillWithLeadingZeros failed, pos 3 is not 0");
	}
	if ((uint8_t)nrbfr[3] != 0x30)
	{
		printf("testFillWithLeadingZeros failed, pos 4 is not 0");
	}
	if ((uint8_t)nrbfr[4] != 0x0)
	{
		printf("testFillWithLeadingZeros failed, string is not zero-terminated");
	}
	printf("******** ending testFillWithLeadingZeros\n");
}


void testInterpolation()
{
	printf("******** starting testInterpolation\n");
	uint32_t nrit=2592000;

	initInterpolators(&interpolators);
	setLampInterpolator(&interpolators,3,2,MODE_REPEATING);
	setColorFramesInterpolation(&interpolators, 0,0,0 ,nrit,INTERPOLATION_LINEAR, 3, 0);
	setColorFramesInterpolation(&interpolators, 234,67,42,3,INTERPOLATION_LINEAR, 3, 1);

	startInterpolators(&interpolators);

	for (uint32_t q=0;q<=nrit;q++)
	{
		updateTask(&interpolators.taskArray[0],lamps);
		//printf("\rStep: %i, r: %i,g: %i, b: %i",q,lamps[3].rgb.r,lamps[3].rgb.g,lamps[3].rgb.b);
		//fflush(stdout);
	}
	if (lamps[3].rgb.r != 234 || lamps[3].rgb.g != 67 || lamps[3].rgb.b != 42)
	{
		printf("failed, color at interpolation end is not as expected");
	}
	printf("******** ending testInterpolation\n");
}

void testStreamRoundtrip()
{
	printf("******** starting testStreamRoundtrip\n");
	initInterpolators(&interpolators);
	setLampInterpolator(&interpolators,3,2,MODE_REPEATING);
	setColorFramesInterpolation(&interpolators, 0,0,0 ,37,INTERPOLATION_LINEAR, 3, 0);
	setColorFramesInterpolation(&interpolators, 234,67,42,3,INTERPOLATION_LINEAR, 3, 1);

	TasksType interpolatorsCopy;
	strcpy(interpolators.name,"UeliDrChnaecht");
	uint8_t* streamout=0;
	toStream(&interpolators,&streamout);
	fromStream((uint16_t*)streamout,&interpolatorsCopy);
	if (!(interpolatorsCopy.taskArrayLength == interpolators.taskArrayLength))
	{
		printf("roundtrip failed: task Array Length not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].Nsteps == interpolators.taskArray[0].Nsteps))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].b_cur == interpolators.taskArray[0].b_cur))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].r_cur == interpolators.taskArray[0].r_cur))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].g_cur == interpolators.taskArray[0].g_cur))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].lamp_nr == interpolators.taskArray[0].lamp_nr))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].state == interpolators.taskArray[0].state))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].stepCnt == interpolators.taskArray[0].stepCnt))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].stepProgressionCnt == interpolators.taskArray[0].stepProgressionCnt))
	{
		printf("property of first Task not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].r == interpolators.taskArray[0].steps[0].r))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].g == interpolators.taskArray[0].steps[0].g))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].b == interpolators.taskArray[0].steps[0].b))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].frames == interpolators.taskArray[0].steps[0].frames))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].interpolation == interpolators.taskArray[0].steps[0].interpolation))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].deltar == interpolators.taskArray[0].steps[0].deltar))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].deltag == interpolators.taskArray[0].steps[0].deltag))
	{
		printf("property of first Task, first Step not identical\n");
	}
	if (!(interpolatorsCopy.taskArray[0].steps[0].deltab == interpolators.taskArray[0].steps[0].deltab))
	{
		printf("property of first Task, first Step not identical\n");
	}
	free(streamout);
	printf("******** ending testStreamRoundtrip\n");
}


void testFlashHeaderWrite()
{
	uint16_t headerdata[8];
	printf("******** starting testFlashHeaderWrite\n");
	char * dummy="lorem ipsum dolor sit amet... ";
	saveData((uint16_t*)dummy,30,FLASH_HEADER_SIZE);

	headerdata[0]=42;
	saveHeader(headerdata,1);

	if(startsWith((char*)(fakeflash)+FLASH_HEADER_SIZE,"lorem ipsum dolor sit amet... ") == 0)
	{
		printf("write to  header has overwritten the body data\n");
	}
	printf("******** ending testFlashHeaderWrite\n");
}

void testFlashWrite()
{
	printf("******** starting testFlashWrite\n");
	srand(3435);
	for (uint8_t q=0;q<200;q++)
	{
		uint16_t* data;
		uint32_t dsize = rand()%100 + 10;
		data = (uint16_t*)malloc(dsize*sizeof(uint16_t));
		for(uint32_t d=0;d<dsize;d++)
		{
			*(data + d) = (uint16_t)(rand() & 0xFFFF);
		}

		uint32_t offset = rand()%(8192-dsize*sizeof(uint16_t));
		offset &= ~(1);
		saveData(data,dsize,offset);
		for(uint16_t c2=0;c2<dsize;c2++)
		{
			if (*((uint16_t*)fakeflash + c2 + (offset>>1)) != *(data + c2))
			{
				printf("characters written do not match: %d vs %d\n",*((uint16_t*)fakeflash + c2 + (offset>>1)),*(data + c2));
			}
		}
		free(data);
	}
	printf("******** ending testFlashWrite\n");
}


int main(int argc,char** argv)
{
	interpolators.taskArray=(TaskType*)interpolatorsArray;
	interpolators.taskArrayLength=N_LAMPS;
	initInterpolators(&interpolators);
	testFlashWrite();
	testFlashHeaderWrite();
	testStreamRoundtrip();
	testInterpolation();
	consoleHandlerHistoryCheck();
	testExpandRange();
	testExpandDescription();
	testRgbCommand();
	demoColorInterpolator();
	testConvertInts();
	testPercentToChar();
	testFillWithLeadingZeros();

	return 0;
}




