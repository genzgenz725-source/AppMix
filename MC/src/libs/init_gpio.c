#include "../../inc/proj_h/main.h"

void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOA->CRL &= ~(0xFFFF);                                     // set 00 00 to first 16bits -> 4 pins PA0-3 for potentiometers data

    GPIOA->CRL &= ~(0xF << 16);                                  // set zeros for DC
    GPIOA->CRL |=  (0x1 << 16);                                  // set DC for SPI displays

    GPIOA->CRL &= ~(0xF << 24);                                  // set zeros for RES
    GPIOA->CRL |=  (0x1 << 24);                                  // set RES for SPI displays

    GPIOA->CRH &= ~(0xFF00F << 12);                              // settings PA11 PA12 PA15 to zero
    GPIOA->CRH |=  (0x11001 << 12);                              // setting PA11 PA12 PA15 to manual sending -> for vibromotors

    GPIOB->CRL &= ~(0xF << 12);                                  // set zeros for SCK
    GPIOB->CRL |=  (0xB << 12);                                  // set SCK (Alternate function)

    GPIOB->CRL &= ~(0xF << 20);                                  // set zeros for MOSI
    GPIOB->CRL |=  (0xB << 20);                                  // set MOSI (Alternate function)

    GPIOB->CRL &= ~(0xF << 16);                                  // set zeros for PB4 (vibromotor)
    GPIOB->CRL |=  (0x1 << 16);                                  // set PB4 for vibromotor

    GPIOB->CRH &= ~(0xFFFF << 16);                               // set zeros for CS for displays
    GPIOB->CRH |=  (0x1111 << 16);                               // set manual sending bytes for CS

    GPIOB->ODR |= (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15); // set CS high
}