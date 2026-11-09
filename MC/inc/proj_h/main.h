#ifndef MAIN_H
#define MAIN_H

#include "../CMSIS/stm32f1xx.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void ADC_Init(void);

extern volatile uint32_t current_volume;
extern volatile uint16_t pins[4];

void USART1_IRQHandler(void);
void INIT_SYSTEM(void);
void send_byte(uint8_t byte);
void send_pkg(const char* data);
void read_all_pins(void);
void APP_DONE(void);

#endif // MAIN_H