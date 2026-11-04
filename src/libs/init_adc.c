#include "../../inc/proj_h/main.h"

void ADC_Init(void) {
    // turn on clock for ADC
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // turn on ADC
    ADC1->CR2 |= ADC_CR2_ADON;
    
    // mini delay
    for (volatile int i = 0; i < 1000; i++);

    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

    // delay for Condensator
    ADC1->SMPR2 |= (0x7 << 0) | (0x7 << 3) | (0x7 << 6) | (0x7 << 9);

    // Sequence cfg
    // Set queue that there would be 4 ADC`s
    ADC1->SQR1 &= ~(0xF << 20);
    ADC1->SQR1 |=  (3 << 20);

    // init every ADC channel by PA0, 1, 2, 3
    ADC1->SQR3 = 0;
    ADC1->SQR3 |= (0 << 0);   // 1-е: канал 0
    ADC1->SQR3 |= (1 << 5);   // 2-е: канал 1
    ADC1->SQR3 |= (2 << 10);  // 3-е: канал 2
    ADC1->SQR3 |= (3 << 15);  // 4-е: канал 3
}