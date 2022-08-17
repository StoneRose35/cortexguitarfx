/**
 * @file interpolators.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief functions for handling an array of Task data structures
 * @version 0.1
 * @date 2021-12-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#include "interpolators.h"
#include "colorInterpolator.h"
#include <stdlib.h>

/**
 * @brief initializes an array of Task data scrutures, frees the dynamically allocated
 * memory for each task of steps is not 0, resets all values to zero except lamp_nr which set to 255
 * 
 * @param tasks the data structure to reset
 */
void initInterpolators(Tasks tasks)
{
	for(uint8_t c=0;c<tasks->taskArrayLength;c++)
	{
		tasks->taskArray[c].Nsteps=0;
		tasks->taskArray[c].b_cur=0;
		tasks->taskArray[c].g_cur=0;
		tasks->taskArray[c].r_cur=0;
		tasks->taskArray[c].lamp_nr=255;
		tasks->taskArray[c].state=0;
		tasks->taskArray[c].stepCnt=0;
		tasks->taskArray[c].stepProgressionCnt=0;
		if (tasks->taskArray[c].steps != 0)
		{
			free(tasks->taskArray[c].steps);
		}
		tasks->taskArray[c].steps=0;
	}
}

/**
 * @brief allocates a new entry within Tasks for a given neopixel element.
 * Returns 1 if a Task for the given neopixel element is already defined and running or if no free index in the Tasks array is found.
 * reuses the first Task array position for the given neopixel number if found.
 * 
 * @param tasks the tasks array holding  alls color progressions/ tasks
 * @param lampNr the neopixel number for which the color progression/task should be initialized
 * @param nsteps the number of steps of the newly defined color progression
 * @param repeating 1 if all tasks should repeat, 0 if none should repeat. There is no option using this function to define only a faction
 *                  of tasks as repeating
 * @return uint8_t 0 if successful, 1 otherwise
 */
uint8_t setLampInterpolator(Tasks tasks,uint8_t lampNr,uint8_t nsteps,uint8_t repeating)
{
	uint8_t taskCreated=0;
	uint8_t firstFreeIndex=255;
	for(uint8_t c=0;c<tasks->taskArrayLength;c++)
	{
		if (tasks->taskArray[c].lamp_nr == lampNr)
		{
			if ((tasks->taskArray[c].state & 0x3) != STATE_STOPPED) // there is an interpolator running for the lamp return an error code
			{
				return 1;
			}
			else
			{
				destroyTask(&tasks->taskArray[c]);
			}
			initTask(&tasks->taskArray[c],nsteps,lampNr);
			tasks->taskArray[c].state &= ~0x4;
			tasks->taskArray[c].state |= repeating << 2;
			taskCreated=1;
		}
		else if (tasks->taskArray[c].steps == 0 && firstFreeIndex == 0xFF)
		{
			firstFreeIndex=c;
		}
	}
	if (taskCreated == 0 )
	{
		if (firstFreeIndex != 0xFF)
		{
			if (tasks->taskArray[firstFreeIndex].steps != 0)
			{
				destroyTask(&tasks->taskArray[firstFreeIndex]);
			}
			initTask(&tasks->taskArray[firstFreeIndex],nsteps,lampNr);
			tasks->taskArray[firstFreeIndex].state &= ~0x4;
			tasks->taskArray[firstFreeIndex].state |= repeating << 2;
			tasks->taskArray[firstFreeIndex].state |= STATE_STARTING;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}

/**
 * @brief gets the indes withing the tasks/color progression array for a certain neopixel nr
 * returns 255 if no entry is found
 * @param tasks the data object holding all tasks
 * @param lampnr the neopixel number for which the index should be returned
 * @return uint8_t the index if found, 255 otherwise 
 */
uint8_t getLampIndex(Tasks tasks,uint8_t lampnr)
{
	for(uint8_t c=0;c<tasks->taskArrayLength;c++)
	{
		if(tasks->taskArray[c].lamp_nr == lampnr)
		{
			return c;
		}
	}
	return 0xFF;
}

/**
 * @brief defines a step for a given neopixel number at a given steps index within a color progression/task
 * 
 * @param tasks the data object holding all tasks
 * @param r Red
 * @param g Green
 * @param b Blue
 * @param frames the number of frames to set 
 * @param interpolation 1 if linear interpolation should be applied, 0 otherwise
 * @param lamp_nr the neopixel number
 * @param step the step index which should be defined
 * @return uint8_t 0 if successful, an error code otherwise
 */
uint8_t setColorFramesInterpolation(Tasks tasks, uint8_t r,uint8_t g,uint8_t b,int32_t frames,uint8_t interpolation, uint8_t lamp_nr,uint8_t step)
{
	uint8_t idx;
	idx=getLampIndex(tasks,lamp_nr);
	if(idx != 0xFF)
	{
		if (step >= tasks->taskArray[idx].Nsteps)
		{
			return ERR_INVALID_STEP;
		}
		tasks->taskArray[idx].steps[step].interpolation = interpolation;
		setColor(&tasks->taskArray[idx],r,g,b,step);
		setFrames(&tasks->taskArray[idx],frames,step);

	}
	return 0;
}

/**
 * @brief starts all tasks
 * 
 * @param tasks the data object holding all tasks
 * @return uint8_t 0 if successful, error code otherwise
 */
uint8_t startInterpolators(Tasks tasks)
{
	for (uint8_t c=0;c<tasks->taskArrayLength;c++)
	{
		if (tasks->taskArray[c].steps != 0)
		{
			start(&tasks->taskArray[c]);
		}
	}
	return 0;
}

/**
 * @brief stops all tasks
 * 
 * @param tasks the data object holding all tasks
 * @return uint8_t 0 if successful, error code otherwise
 */
uint8_t stopInterpolators(Tasks tasks)
{
	for (uint8_t c=0;c<tasks->taskArrayLength;c++)
	{
		stop(&tasks->taskArray[c]);
	}
	return 0;
}

/**
 * @brief serializes the Tasks data structure to a byte array
 * 
 * @param t the data object holding all tasks
 * @param resultStream the pointer to the byte output stream passed by reference
 * @return uint32_t the size in bytes of the Tasks data structure
 */
uint32_t toStream(Tasks t,uint8_t** resultStream)
{
	uint32_t tasksSize = 0;
	uint32_t offset=0,offsetStream = 0;
	uint32_t colorStepArraySize;

	/*
	 * SIZE CALCULATION
	 */
	tasksSize += sizeof(TasksType) - sizeof(void*); // excluding the pointer to the array
	for(uint8_t c=0;c<t->taskArrayLength;c++)
	{
		tasksSize += sizeof(TaskType) - sizeof(void*); // again removing the pointer
		tasksSize += t->taskArray[c].Nsteps*sizeof(ColorStepType);
	}
	if ((tasksSize & 0x1) == 0x1) // add a zero byte to get an integer half-word size
	{
		tasksSize++;
	}

	/*
	 * MEMORY ALLOCATION
	 */
	*resultStream=(uint8_t*)malloc(tasksSize);


	// store tasks header information (the nubmer of TaskType's and the name)
	for(uint8_t c=0;c<sizeof(TasksType)-sizeof(void*);c++)
	{
		*(*resultStream + offsetStream++)=*((uint8_t*)t+offset++);
	}
	// store TaskType's
	for(uint8_t c=0;c<t->taskArrayLength;c++)
	{
		// loop through TaskType
		offset = 0;
		for (uint8_t c2=0;c2<sizeof(TaskType)-sizeof(void*);c2++)
		{
			*(*resultStream + offsetStream++) = *((uint8_t*)(t->taskArray + c)+offset++);
		}
		// store ColorStepTypes
		colorStepArraySize = t->taskArray[c].Nsteps*sizeof(ColorStepType);
		for (uint8_t c3=0;c3<colorStepArraySize;c3++)
		{
			*(*resultStream + offsetStream++) = *((uint8_t*)t->taskArray[c].steps + c3);
		}
	}

	return tasksSize;
}

/**
 * @brief converts a halfword stream into a Tasks data structure
 * 
 * @param stream the data stream holding the information to build a
 * @param t the Tasks data structure to which the stream data is converted to 
 */
void fromStream(uint16_t* stream,Tasks t)
{
	uint32_t streamOffset = 0;
	uint8_t* byteStream=(uint8_t*)stream;
	uint16_t colorStepArraySize=0;
	ColorStepType* cstep;
	for(uint8_t c=0;c<sizeof(TasksType)-sizeof(void*);c++)
	{
		*((uint8_t*)t + c) = *(byteStream+streamOffset++);
	}
	TaskType* tarr=(TaskType*)malloc((t->taskArrayLength)*(sizeof(TaskType)));
	t->taskArray=tarr;
	for (uint8_t c2=0;c2 < t->taskArrayLength;c2++)
	{
		t->taskArray[c2].steps=0;
		for (uint8_t c1=0;c1 < sizeof(TaskType)-sizeof(void*);c1++)
		{
			*((uint8_t*)(t->taskArray+c2) +  c1) = *(byteStream+streamOffset++);
		}
		colorStepArraySize = t->taskArray[c2].Nsteps*sizeof(ColorStepType);
		if(colorStepArraySize > 0)
		{
			cstep=(ColorStepType*)malloc(colorStepArraySize);
			for(uint16_t c5=0;c5<colorStepArraySize;c5++)
			{
				*((uint8_t*)(cstep)+c5) = *(byteStream+streamOffset++);
			}
			t->taskArray[c2].steps = cstep;
		}
		else
		{
			t->taskArray[c2].steps = 0;
		}
	}
}

