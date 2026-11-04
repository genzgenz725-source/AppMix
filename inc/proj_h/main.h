#ifndef MAIN_H
#define MAIN_H

#include "../CMSIS/stm32f1xx.h"
#include <stdint.h>
#include <stdio.h>

void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void ADC_Init(void);

extern volatile uint32_t current_volume;
extern volatile uint16_t pins[4];

void read_all_pots(uint32_t* pots);
bool APP_DONE();

#endif // MAIN_H