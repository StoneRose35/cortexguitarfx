/*
 * led_timer.h
 *
 *  Created on: Jul 27, 2021
 *      Author: philipp
 */




#ifndef LED_TIMER_H_
#define LED_TIMER_H_

#include <stdint.h>
#include "system.h"

#ifdef STM32
typedef struct {
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SMCR;
	volatile uint32_t DIER;
	volatile uint32_t SR;
	volatile uint32_t EGR;
	volatile uint32_t CCMR1;
	volatile uint32_t CCMR2;
	volatile uint32_t CCER;
	volatile uint32_t CNT;
	volatile uint32_t PSC;
	volatile uint32_t ARR;
	volatile uint32_t reserved;
	volatile uint32_t CCR1;
	volatile uint32_t CCR2;
	volatile uint32_t CCR3;
	volatile uint32_t CCR4;
	volatile uint32_t reserved1;
	volatile uint32_t DCR;
	volatile uint32_t DMAR;
} TIM23TypeDef;

typedef struct {
	volatile uint32_t CCR;
	volatile uint32_t CNDTR;
	volatile uint32_t CPAR;
	volatile uint32_t CMAR;
	volatile uint32_t reserved;
} DMAChannelTypeDef;

typedef struct {
	volatile uint32_t ISR;
	volatile uint32_t IFCR;
	DMAChannelTypeDef CHANNEL[7];
} DMATypeDef;

typedef struct {
	volatile uint32_t MODER;
	volatile uint32_t OTYPER;
	volatile uint32_t OSPEEDR;
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFRL;
	volatile uint32_t AFRH;
	volatile uint32_t BRR;

} GPIOTypeDef;

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t APB2RSTR;
	volatile uint32_t APB1RSTR;
	volatile uint32_t AHBENR;
	volatile uint32_t APB2ENR;
	volatile uint32_t APB1ENR;
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
	volatile uint32_t AHBRSTR;
	volatile uint32_t CFGR2;
	volatile uint32_t CFGR3;

} RCCTypeDef;

typedef struct {
	volatile uint32_t ACR;
	volatile uint32_t KEYR;
	volatile uint32_t OPTKEYR;
	volatile uint32_t SR;
	volatile uint32_t CR;
	volatile uint32_t AR;
	volatile uint32_t OBR;
	volatile uint32_t WRPR;
} FlashTypeDef;

typedef struct {
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t OAR1;
	volatile uint32_t OAR2;
	volatile uint32_t TIMINGR;
	volatile uint32_t TIMEOUTR;
	volatile uint32_t ISR;
	volatile uint32_t ICR;
	volatile uint32_t PECR;
	volatile uint32_t RXDR;
	volatile uint32_t TXDR;
} I2CTypeDef;

// GPIO bits
#define INPUT (0)
#define OUTPUT (1)
#define AF (2)
#define ANALOG (3)

// RCC bits
#define PLLON (24)
#define PLLRDY (25)
#define CSSON (19)
#define HSIRDY (1)
#define HSION (0)
#define PLLNODIV (31)
#define PLLMULT (18)
#define PLLSRC (15)
#define PPRE2 (11)
#define PPRE1 (8)
#define SWS (2)
#define SW (0)
#define I2C1EN (21)

// Flash bits
#define LATENCY (0)
#define FLASH_BSY (0)
#define FLASH_PER (1)
#define FLASH_STRT (6)
#define FLASH_EOP (5)
#define FLASH_LOCK (7)
#define FLASH_PG (0)

#define CEN (0)
#define OC1M (4)
#define OC1PE (3)
#define OC1FE (2)
#define ARPE (7)
#define TIM2EN (0)
#define TIM3EN (1)
#define IOPAEN (17)
#define IOPBEN (18)
#define DMA1EN (0)
#define PINA0POS (0)
#define PINA9POS (9)
#define UIE (0)
#define CC1DE (9)
#define UDE (8)

// DMA CCR bits
#define MEM2MEM (14)
#define PL (12)
#define MSIZE (10)
#define PSIZE (8)
#define MINC (7)
#define PINC (6)
#define CIRC (5)
#define DIR (4)
#define TEIE (3)
#define HTIE (2)
#define TCIE (1)
#define EN (0)

// I2C bits
#define RXIE (2)
#define OA1EN (15)

#define RCC ((RCCTypeDef*)0x40021000UL)
#define GPIOA ((GPIOTypeDef*)0x48000000UL)
#define GPIOB ((GPIOTypeDef*)0x48000400UL)
#define TIM2 ((TIM23TypeDef*)0x40000000UL)
#define TIM3 ((TIM23TypeDef*)0x40000400UL)
#define DMA ((DMATypeDef*)0x40020000UL)
#define NVIC_ISER0 ((volatile uint32_t*)0xE000E100UL)
#define NVIC_ISER1 ((volatile uint32_t*)0xE000E104UL)
#define NVIC_ICER1 ((volatile uint32_t*)0xE000E184UL)
#define FLASH ((FlashTypeDef*)0x40022000UL)
#define I2C1 ((I2CTypeDef*)0x4000400UL)

// if timer 2 is not running, i.e. data transfer is over
#define READY_TO_SEND (TIM2->CR1 & 1) != 1
// data has been sent, now just waiting for the remaining time to elapse
#define WAIT_STATE TIM2->ARR > WS2818_CNT
#endif

#ifdef HARDWARE
void initNeopixels();
void decompressRgbArray(RGBStream * frame,uint8_t length);
void sendToLed();
uint8_t getSendState();
void setSendState(uint8_t s);

extern uint32_t clr_cnt;
extern uint32_t bit_cnt;
extern uint8_t * rawdata_ptr;

#define SEND_STATE_INITIAL 0
#define SEND_STATE_DATA_READY 5
#define SEND_STATE_RTS 1
#define SEND_STATE_SENDING 2
#define SEND_STATE_SENT 3
#define SEND_STATE_BUFFER_UNDERRUN 4
#endif

void setEngineState(uint8_t); // enginestate gets called by the task manager therefore it must be mocked for the hardware independent tests
uint8_t getEngineState();

#endif
