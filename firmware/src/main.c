#include <stdint.h>
#include "stm32g4xx.h"
// pulls in ST's chip definitions

static void delay(void)
{
    for (volatile uint32_t i = 0; i < 100000; i++) {
    }
}

uint32_t cpp_selftest(void);

int main(void)
{
    volatile uint32_t r = cpp_selftest();
    (void)r;   // tells the compiler "I'm keeping r on purpose even if unused"

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;      // 1. clock on for GPIOA
    (void)RCC->AHB2ENR;                       //    read back: gives the clock a moment to start

    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;    // 2. clear PA5's mode bits (was 11 = analog)
    GPIOA->MODER |= GPIO_MODER_MODE5_0;       //    set them to 01 = output

    while (1) {
        GPIOA->ODR ^= GPIO_ODR_OD5;           // 3. toggle PA5. , need this line number so we can look at the register before and after toggling 
        delay();                              //    wait (delay loop)
    }
}