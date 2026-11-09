#include "../../inc/proj_h/main.h"

void SystemClock_Config(void) {
    // turn on quartz and waiting for it
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0);

    // set latency 2 tact for flash
    FLASH->ACR |= FLASH_ACR_LATENCY_2 | FLASH_ACR_PRFTBE;

    // mulltiply tacts by 9 times for PLL
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;

    // turn PLL on
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    // devided for buses AHB(PLL/2=36mhz) APB2(full 72) APB1(full 72)
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_HPRE_DIV1;

    // PLL selected as system clock
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    SystemCoreClockUpdate();
}