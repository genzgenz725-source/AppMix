#include "../inc/proj_h/main.h"

volatile uint32_t current_volume;
volatile uint16_t pins[4];
volatile bool ready_apps = 0;
const char startPkg[15] = "0 210214301131";
uint8_t rx_buffer[64];
volatile uint8_t rx_index = 0;

void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        uint8_t byte = USART1->DR;
        
        rx_buffer[rx_index++] = byte;
        
        if (byte == '\n') {
            rx_buffer[rx_index] = '\0';
            rx_index = 0;
        }
    }
}

void INIT_SYSTEM(void) {
    SystemClock_Config();   // clock cfg
    GPIO_Init();            // pins cfg
    UART_Init();            // uart cfg
    ADC_Init();             // ADC cfg
}

void send_byte(uint8_t byte) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = byte;
}

void send_pkg(const char* data) {
    while (*data != '\0') {
        send_byte(*data);
        data++;
    }
}

void read_all_pins(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;           // starting conversion

    for (int i = 0; i < 4; i++) {
        while (!(ADC1->SR & ADC_SR_EOC));   // waiting for ready ADC
        
        pins[i] = ADC1->DR;                 // init [i] by value 0-4096 from ADC
    }
}

void APP_DONE(void) {
    send_pkg(startPkg);
    // sending to pc init pack, and waiting for logos of apps

    while (!ready_apps);
}

int main(void) {
    INIT_SYSTEM();
    
    APP_DONE();

    while (1) {
        read_all_pins();
    }

    return 0;
}