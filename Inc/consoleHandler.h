/*
 * consoleHandler.h
 *
 *  Created on: 15.09.2021
 *      Author: philipp
 */
#include "system.h"
#include "bufferedInputStructs.h"

#ifndef CONSOLEHANDLER_H_
#define CONSOLEHANDLER_H_


#define CONSOLE_PREFIX "Microsys:"
#define CONSOLE_MAX_PATHLENGTH 256

void initConsole(Console);
char* onCharacterReception(BufferedInput,uint8_t);
void clearCommandBuffer(Console,uint8_t,char*);
void clearOutBuffer(Console);
void copyCommand(uint8_t,uint8_t,char*); // copies a command from index src to index target
void copyCommandBetweenArrays(uint8_t,uint8_t,char*,char*);

void addToPath(char * folder);
void removeLastPath();

#endif /* CONSOLEHANDLER_H_ */
