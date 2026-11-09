#include "../../inc/proj_h/main.h"

void UART_Init(void) {
    // turn on clock for uart
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // write 100111 0001 in BRR 0,4,5,6,9 bits: 0x0271: we need 115200 baud, formula: HZ / (16 * baud) = value we need to write
    USART1->BRR = (39 << 4) | 1;    // 39 full value << 4(const), and mull. float part by 16(its 1 in our case)

    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

// send all bytes by UART
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = (ptr[i] & 0xFF);
    }
    return len;
}