/*
 * memoryAccess.h
 *
 *  Created on: 05.11.2021
 *      Author: philipp
 */

#ifndef MEMORYACCESS_H_
#define MEMORYACCESS_H_

uint16_t saveData(uint16_t * data,uint32_t size,uint32_t offset);
uint16_t saveHeader(uint16_t * data,uint32_t size);

#endif /* MEMORYACCESS_H_ */
