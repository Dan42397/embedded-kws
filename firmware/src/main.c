#include <stdint.h>
#include "stm32g4xx.h" // device header: pulls in ST's chip definitions such as GPIOA->MODER,



//The loop burns CPU cycles, creating a rough pause between LED toggles. 
//Its duration depends on the MCU clock and compiler settings, so it is not a precise timer. 
//For a reliable delay use a hardware timer 
static void delay(void)
{
    for (volatile uint32_t i = 0; i < 100000; i++) {
        //volatile tells the compiler not to optimize away the counter, even when loop body is empty.
    }
}

uint32_t cpp_selftest(void);

int main(void)
{
    volatile uint32_t r = cpp_selftest();
    (void)r;   // tells the compiler "I'm keeping r on purpose even if unused"


    //|= means “OR this mask into the register and store the result.” 
    //It sets that bit to 1 while leaving the other bits unchanged.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;      // 1. turn on clock for GPIOA (Port A clock)
    (void)RCC->AHB2ENR;                       //    read back: gives the clock a moment to start

    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;    // 2. clear PA5's mode bits (was 11 = analog)(sets bits 11 and 10 to 00)
    GPIOA->MODER |= GPIO_MODER_MODE5_0;       //    set them to 01 = output, (make PA5 an output)

    while (1) {
        //XOR so that if the bit is 0 it becomes 1, and if it is 1 it becomes 0.
        GPIOA->ODR ^= GPIO_ODR_OD5;           // 3. toggle PA5. , need this line number so we can look at the register before and after toggling 
        delay();                              //    wait (delay loop)
    }
}