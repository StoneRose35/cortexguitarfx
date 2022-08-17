#include "colorInterpolator.h"
#include <stdio.h>
#include "system.h"

#define N_TASKS 20

void demoColorInterpolator()
{

	RGBStream lampdataArray[N_TASKS];
	RGBStream * lampdata = lampdataArray;

	void* tasksArray[N_TASKS*sizeof(TaskType)];

	TaskType * tasks = (TaskType*)tasksArray;

	// manually init data

	for (uint8_t c=0;c<N_TASKS;c++)
	{
		(tasks + c)->lamp_nr = c;
		initTask(tasks +c,1,c);
	}
	// go from red to blue in 45 frames for lamp 5 (0 is first lamp
	initTask(tasks +5,2,5);

	setColor(tasks+5, 240,0,0,0);
	setColor(tasks+5, 0,0,234,1);
	setFrames(tasks+5,45,0);
	(tasks + 5)->steps[0].interpolation= 1;
	start(tasks+5);

	uint8_t running = 1;
	while (running != 0)
	{
		running = 0;
		for(uint8_t c=0;c<N_TASKS;c++)
		{
			updateTask(tasks+c,lampdata);
			if ((tasks+c)->state != 0)
			{
				running = 1;
			}
		}
	}

	for(uint8_t c=0;c<N_TASKS;c++)
	{
		stop(tasks+c);
	}
	printf("finished\n");
}
