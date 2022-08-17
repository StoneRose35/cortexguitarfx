/**
 * @file colorInterpolator.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief functions for describing the color progression over time of a single neopixel element
 * @version 0.1
 * @date 2021-12-21
 * 
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#ifdef HARDWARE
#include "system.h"
#endif
#include "colorInterpolator.h"


/**
 * @brief allocates memory and zero-initializes a structure holding a color progression of a single neopixel element
 * 
 * @param t the Task which has to be initialized, don't initialize Tasks which are already initialized, call destroyTask first
 * @param nsteps the number of steps/individual colors the Task contains
 * @param lampnr the element within the neopixel sequences, start with zero un to N_LAMPS - 1
 */
void initTask(Task t,uint8_t nsteps,uint8_t lampnr)
{
	t->lamp_nr=lampnr;
	t->Nsteps=nsteps;
	t->stepCnt=0;
	t->stepProgressionCnt=0;
	t->state=0;
	if (nsteps > 0)
	{
		void* csArray = malloc(nsteps*sizeof(ColorStepType));
		t->steps = (ColorStepType*)csArray;
	}
	else
	{
		t->steps = 0;
	}
	for(uint8_t c=0;c<nsteps;c++)
	{
		t->steps[c].r=0;
		t->steps[c].g=0;
		t->steps[c].b=0;
		t->steps[c].deltar=0;
		t->steps[c].deltag=0;
		t->steps[c].deltab=0;
		t->steps[c].frames=1;
		t->steps[c].interpolation=0;
	}
}

/**
 * @brief frees the allocated memory of the task and sets all parameters to zero except
 * `lamp_nr` which is set to 255 to indicate that the Task is destroyed  
 * doesn't lead to error if an already destroyed task is destroyed again
 * has to be called prior to defining a new task at a certain position in the tasks array
 *
 * @param t the Task to destry
 */
void destroyTask(Task t)
{
	t->lamp_nr=255;
	t->Nsteps=0;
	t->stepCnt=0;
	t->stepProgressionCnt=0;
	t->state=0;
	if(t->steps != 0)
	{
		free(t->steps);
	}
	t->steps=0;
}

/**
 * @brief sets the color of a given step
 * 
 * @param t the Task being manipulated
 * @param r Red
 * @param g Green
 * @param b Blue
 * @param idx the index, can be within 0 to Task.Nsteps
 */
void setColor(Task t,uint8_t r,uint8_t g, uint8_t b,uint8_t idx)
{
	t->steps[idx].r = r;
	t->steps[idx].g = g;
	t->steps[idx].b = b;

	if (idx > 0)
	{
		t->steps[idx-1].deltar = ((t->steps[idx].r - t->steps[idx-1].r) << 23)/t->steps[idx-1].frames;
		t->steps[idx-1].deltag = ((t->steps[idx].g - t->steps[idx-1].g) << 23)/t->steps[idx-1].frames;
		t->steps[idx-1].deltab = ((t->steps[idx].b - t->steps[idx-1].b) << 23)/t->steps[idx-1].frames;
	}

	if (idx < t->Nsteps-1)
	{
		t->steps[idx].deltar = ((t->steps[idx+1].r - t->steps[idx].r) << 23)/t->steps[idx].frames;
		t->steps[idx].deltag = ((t->steps[idx+1].g - t->steps[idx].g) << 23)/t->steps[idx].frames;
		t->steps[idx].deltab = ((t->steps[idx+1].b - t->steps[idx].b) << 23)/t->steps[idx].frames;
	}
	else if (idx == t->Nsteps-1 && (t->state & (1 << STATE_REPEATING))== (1 << STATE_REPEATING))
	{
		t->steps[idx].deltar = ((t->steps[0].r - t->steps[idx].r) << 23)/t->steps[idx].frames;
		t->steps[idx].deltag = ((t->steps[0].g - t->steps[idx].g) << 23)/t->steps[idx].frames;
		t->steps[idx].deltab = ((t->steps[0].b - t->steps[idx].b) << 23)/t->steps[idx].frames;
	}
}

/**
 * @brief Set the duration in frames of a given step
 * 
 * @param t the Task to edit
 * @param nframes the new duration in frames
 * @param idx the steps number to edit, goes from 0 to Task.Nsteps
 */
void setFrames(Task t,int32_t nframes,uint8_t idx)
{
	t->steps[idx].frames = nframes;
	if (idx < t->Nsteps-1)
	{
		t->steps[idx].deltar = ((t->steps[idx+1].r - t->steps[idx].r) << 23)/t->steps[idx].frames;
		t->steps[idx].deltag = ((t->steps[idx+1].g - t->steps[idx].g) << 23)/t->steps[idx].frames;
		t->steps[idx].deltab = ((t->steps[idx+1].b - t->steps[idx].b) << 23)/t->steps[idx].frames;
	}
	else if (idx == t->Nsteps-1 && (t->state & (1 << STATE_REPEATING))== (1 << STATE_REPEATING))
	{
		t->steps[idx].deltar = ((t->steps[0].r - t->steps[idx].r) << 23)/t->steps[idx].frames;
		t->steps[idx].deltag = ((t->steps[0].g - t->steps[idx].g) << 23)/t->steps[idx].frames;
		t->steps[idx].deltab = ((t->steps[0].b - t->steps[idx].b) << 23)/t->steps[idx].frames;
	}

}

/**
 * @brief indicates the system to update the colors at each frame tick, first resets the counters
 * 
 * @param t the Task to manipulate
 */
void start(Task t)
{
	t->stepCnt=0;
	t->stepProgressionCnt=0;
	t->state &= ~STATE_RUNNING;
	t->state |= STATE_STARTING;

}

/**
 * @brief indicates the system to stop updating the colors at frame tick events. Resets both the stepCnt and stepProgressionCnt
 * 
 * @param t the Task to manipulate
 */
void stop(Task t)
{
	t->stepCnt=0;
	t->stepProgressionCnt=0;
	t->state &= ~(STATE_STARTING | STATE_RUNNING);
}

/**
 * @brief indicates the system to stop updating the colors at frame tick events. Doesn't update the counters.
 * 
 * @param t the Task to manipulate
 */
void pause(Task t)
{
	t->state &= ~(STATE_STARTING | STATE_RUNNING);
}

/**
 * @brief resumes the color animation, the same as start() except that in this case the counters aren't reset
 * 
 * @param t the Task to manipulate
 */
void resume(Task t)
{
	t->state &= ~STATE_STARTING;
	t->state |= STATE_RUNNING;
}

/**
 * @brief Set the neopixel array index which this Task or Color Progression should handle
 * 
 * @param t the Task to manipulate
 * @param nr the neopixel array index (from 0 to N_LAMPS -1)
 */
void setLampNr(Task t, uint8_t nr)
{
	t->lamp_nr=nr;
}

/**
 * @brief updates the current color ot the task, should be called by the system once at each frame tick
 * 
 * @param t the Task to manipulate
 * @param lampdata the output array/ framebuffer holding the the colors to be displayed at the next frame tick
 */
void updateTask(Task t,RGBStream * lampdata)
{
	if ((t->state & 0x3) == STATE_RUNNING && t->steps != 0) // running
	{
		if (t->stepProgressionCnt == t->steps[t->stepCnt].frames)
		{
			t->stepCnt++;
			t->stepProgressionCnt = 0;
			if(t->stepCnt==t->Nsteps)
			{
				if((t->state & (1 << STATE_REPEATING)) == (1 << STATE_REPEATING))
				{
					t->stepCnt = 0;
				}
				else
				{
					// stop the task
					t->stepCnt--;
					t->state &= ~(3 << 0);
				}
			}
			t->r_cur = t->steps[t->stepCnt].r << 23;
			t->g_cur = t->steps[t->stepCnt].g << 23;
			t->b_cur = t->steps[t->stepCnt].b << 23;
		}
		else
		{

			if (t->steps[t->stepCnt].interpolation == 1)
			{
				t->r_cur += t->steps[t->stepCnt].deltar;
				t->g_cur += t->steps[t->stepCnt].deltag;
				t->b_cur += t->steps[t->stepCnt].deltab;
			}
		}
		lampdata[t->lamp_nr].rgb.r = (t->r_cur >> 23) & 0xFF;
		lampdata[t->lamp_nr].rgb.g = (t->g_cur >> 23) & 0xFF;
		lampdata[t->lamp_nr].rgb.b = (t->b_cur >> 23) & 0xFF;
		t->stepProgressionCnt++;
	}
	else if ((t->state & 0x3) == STATE_STARTING)
	{
		t->state &= ~0x3;
		t->state |= STATE_RUNNING;
		t->r_cur = t->steps[t->stepCnt].r << 23;
		t->g_cur = t->steps[t->stepCnt].g << 23;
		t->b_cur = t->steps[t->stepCnt].b << 23;

		lampdata[t->lamp_nr].rgb.r = (t->r_cur >> 23) & 0xFF;
		lampdata[t->lamp_nr].rgb.g = (t->g_cur >> 23) & 0xFF;
		lampdata[t->lamp_nr].rgb.b = (t->b_cur >> 23) & 0xFF;
		t->stepProgressionCnt++;

	}
}

/**
 * @brief 
 * gets the current Task Progression as a number from 0 to 1
 *
 * @param t the Task whose progression should be returned
 * @return float 
 */
float getProgression(Task t)
{
	uint32_t totalFrames=0,passedFrames=0;
	for (uint8_t c=0;c< t->Nsteps;c++)
	{
		totalFrames += t->steps[c].frames;
		if (c < t->stepCnt)
		{
			passedFrames += t->steps[c].frames;
		}
		else if (c == t->stepCnt)
		{
			passedFrames += t->stepProgressionCnt;
		}
	}
	return (float)passedFrames/totalFrames;
}

/**
 * @brief returs the size in bytes of the color progression / task
 * 
 * @param t the Task whose color progression should be calculated
 * @return uint32_t size in bytes
 */
uint32_t getSize(Task t)
{
	return sizeof(TaskType) + t->Nsteps*sizeof(ColorStepType);
}


