/**
 * @file colorInterpolator.h
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief  header for colorInterpolator.c
 * @version 0.1
 * @date 2021-12-21
 * 
 * 
 */
#ifndef COLORINTERPOLATOR_H_
#define COLORINTERPOLATOR_H_

#include <stdint.h>
#include "system.h"

#define STATE_STOPPED 0 //!< the color progression is stopped
#define STATE_STARTING 1 //!< the color progression ha been started but has no been updated by the system yet
#define STATE_RUNNING 2 //!< the color progression has been updated at least once after having started
#define STATE_REPEATING 2 //!< position within state bit array defining if the color progression should be repeating or not
#define INTERPOLATION_CONSTANT 0 //!< for Step.interpolation, bit 0:  color is held
#define INTERPOLATION_LINEAR 1 //!< for Step.interpolation, bit 1:  color is gradually changin to the next one
#define MODE_ONE_SHOT 0 //!< unused
#define MODE_REPEATING 1 //!<unused

/**
 * @brief holds a single step in a color progression, defines how long either the given color should be held or how
 * long the transition from the given color to the next one takes
 * 
 */
typedef struct {
	uint8_t r; //!< Red
	uint8_t g; //!< Green
	uint8_t b; //!< Blue
	uint8_t interpolation; //!<  bit array, position 0: interpolation mode (set: linear, unset: constant)
	int32_t frames; //!< the duration in frames, the duration in seconds would be @see frames / @see FRAMERATE
	int32_t deltar; //!< the change of the Red value per frame when linear interpolation is set, this value is computed during initialization
	int32_t deltag; //!< see deltar
	int32_t deltab; //!< see deltar

} ColorStepType;

/**
 * @brief pointer to ColorStepType, defined for convenience
 * 
 */
typedef ColorStepType *ColorStep;

/**
 * @brief hold the data for an entire color progression for a single neopixel element
 * 
 */
typedef struct {
	uint32_t stepProgressionCnt; //!< hold the current progression within the steps currently executed, goes from 0 to the length of @see ColorStepType.frames. 
	                             //!< this value is handled by the system 
	uint8_t Nsteps; //!< the number of steps the progression contains
	uint8_t stepCnt; //!< the steps currently executed, goes from 0 to Nsteps-1, this value is handled by the system
	uint8_t lamp_nr; //!< the neopixel element to which the color progression applies, goes from 0 to @see N_LAMPS
	uint8_t state; //!< bits 0-1: 0 stopped, 1 starting, 2 running, bit2: repeating if set
	uint32_t r_cur; //!< current red value, this value is handled by the system
	uint32_t b_cur; //!< current blue value, this value is handled by the system
	uint32_t g_cur; //!< current green value, this value is handled by the system
	ColorStepType * steps; //!< the array of steps, allocated dynamically
} TaskType;

/**
 * @brief pointer to TaskType, defined for convenience
 * 
 */
typedef TaskType *Task;


void initTask(Task t,uint8_t nsteps,uint8_t lampnr);

void destroyTask(Task t);

void setColor(Task t,uint8_t r,uint8_t g, uint8_t b,uint8_t idx);

void setFrames(Task t,int32_t nframes,uint8_t idx);

void start(Task t);

void stop(Task t);

void pause(Task t);

void resume(Task t);

void setLampNr(Task,uint8_t);

void updateTask(Task t,RGBStream * lampdata);

float getProgression(Task t);

uint32_t getSize(Task t);


#endif /* COLORINTERPOLATOR_H_ */
