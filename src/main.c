#include "../inc/proj_h/main.h"

volatile uint32_t current_volume;
volatile uint16_t pins[4];
const char startPkg[] = "";

inline void INIT_ALL() {
    SystemClock_Config();   // clock cfg
    GPIO_Init();            // pins cfg
    UART_Init();            // uart cfg
    ADC_Init();             // ADC cfg
}

void read_all_pins() {
    ADC1->CR2 |= ADC_CR2_SWSTART;           // starting conversion

    for (int i = 0; i < 4; i++) {
        while (!(ADC1->SR & ADC_SR_EOC));   // waiting for ready ADC
        
        pins[i] = ADC1->DR;                 // init [i] by value 0-4096 from ADC
    }
}

_Bool APP_DONE() {
    // sending to pc init pack, and waiting for logos of apps

    return 1;
}

int main(void) {
    INIT_ALL();
    
    APP_DONE();

    while (1) {
        read_all_pins();
    }

    return 0;
}