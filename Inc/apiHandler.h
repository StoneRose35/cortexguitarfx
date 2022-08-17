/*
 * apiHandler.h
 *
 *  Created on: 01.11.2021
 *      Author: philipp
 */
#include "system.h"
#include "bufferedInputStructs.h"

#ifndef APIHANDLER_H_
#define APIHANDLER_H_




void initApi(Api api);
char* onByteReception(BufferedInput binput,uint8_t c);

#endif /* APIHANDLER_H_ */
