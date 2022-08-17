/*
 * interpolators.h
 *
 *  Created on: 21.10.2021
 *      Author: philipp
 */

#ifndef INTERPOLATORS_H_
#define INTERPOLATORS_H_

#define ERR_INVALID_STEP 128 //!< return code indicating an invalid step

#include "colorInterpolator.h"

/**
 * @brief data structure for a complete set of Task data structured needed to show an animation
 * 
 */
typedef struct {
	uint8_t taskArrayLength; //!< the number of Tasks in the data structure
	char name[32]; //!< name of the animation
	TaskType* taskArray; //!< the array of individual Tasks

} TasksType;

/**
 * @brief pointer to TasksType, defined for convenience
 * 
 */
typedef TasksType* Tasks;

void initInterpolators(Tasks tasks);
uint8_t getLampIndex(Tasks,uint8_t);
uint8_t setLampInterpolator(Tasks tasks,uint8_t,uint8_t,uint8_t);
uint8_t setColorFramesInterpolation(Tasks, uint8_t,uint8_t,uint8_t,int32_t, uint8_t,uint8_t, uint8_t);
uint8_t startInterpolators(Tasks tasks);
uint8_t stopInterpolators(Tasks tasks);

uint32_t toStream(Tasks,uint8_t**);
void fromStream(uint16_t* stream,Tasks t);

#endif /* INTERPOLATORS_H_ */
