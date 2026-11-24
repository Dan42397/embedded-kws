#include <stdint.h>
#include "stm32g4xx.h" // device header: ST's chip definitions (RCC, GPIOA, TIM2 struct pointers and bit masks)


uint32_t cpp_selftest(void);


/* Port A clock on; PA5 routed to TIM2 channel 1 via alternate function AF1 (step 3.3).
 * PA5 drives the Nucleo's green LED (LD2). Behind every pin is a multiplexer: MODER picks
 * GPIO / analog / alternate function, and AFRL picks which alternate function.
 * TIM2_CH1 is AF1 on PA5 (DS12288 §4.11).
 */
static void gpio_init(void)
{
    //|= means "OR this mask into the register and store the result."
    //It sets that bit to 1 while leaving the other bits unchanged.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;      // 1. turn on clock for GPIOA (Port A clock)
    (void)RCC->AHB2ENR;                       //    read back: makes sure the write has landed before GPIOA is touched


    /* 2. Select AF1 for PA5. AFR[0] is AFRL: pins 0-7, 4 bits each, so PA5
     *    is bits 23:20. Written before MODER so the pin never passes through AF0.
     *    RM0440 §9.4.9 GPIOx_AFRL */
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL5_Msk;          // clear PA5's alternate-function field (reset value 0000 = AF0)
    GPIOA->AFR[0] |=  (1u << GPIO_AFRL_AFSEL5_Pos);  // set PA5 to AF1 = TIM2_CH1 (bits 23:20 = 0001)

    // AFR[0] — which register. The GPIO port has two alternate-function registers: AFRL (pins 0–7) and AFRH (pins 8–15).
    // The CMSIS struct stores them as a two-element array, AFR[0] = AFRL and AFR[1] = AFRH. PA5 is pin 5, in the 0–7 range, so you want AFR[0].
    // The 0 means "the low register," not AF0 — it has nothing to do with which alternate function you pick.


    /* 3. PA5 mode = 10 (alternate function): the pin now follows TIM2_CH1 instead of ODR.
     *    RM0440 §9.4.1 GPIOx_MODER */
    // MODER is the GPIO port mode register. Each pin has two bits that select its mode: 00 = input, 01 = output, 10 = alternate function, 11 = analog.
    // PA5 is pin 5, so its bits are 11:10. CMSIS defines a mask and a position for each pin's mode bits: the mask is 0b11 shifted left by
    // (pin number × 2), the position is (pin number × 2). Use these to clear and set the bits without affecting other pins.
    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;    // clear PA5's mode bits (reset value 11 = analog; bits 11:10 -> 00)
    GPIOA->MODER |=  GPIO_MODER_MODE5_1;      // set PA5 to alternate-function mode (bit 11 = 1, bit 10 = 0 -> 10)

}

/* TIM2 channel 1 as a PWM source (step 3.2).
 *
 *   timer clock = 16 MHz / (PSC + 1) = 1 MHz
 *   period      = (ARR + 1) ticks    = 1000 ticks = 1 ms  -> 1 kHz
 *   duty        = CCR1 / (ARR + 1)   = 250 / 1000 = 25 %
 *
 * The output reaches PA5 because gpio_init() routes the pin to TIM2_CH1 (step 3.3).
 * Register details: RM0440 §30, TIM2/3/4/5.
 */
static void pwm_init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;      // 1. turn on clock for TIM2 (gives TIM2 power; the counter does not start yet)
    (void)RCC->APB1ENR1;                       //    read back: makes sure the write has landed before TIM2 is touched

    TIM2->PSC = 16 - 1;                        // 2. prescaler: divide by 16 -> timer clock = 16 MHz / 16 = 1 MHz = 1,000,000 ticks per second (how fast the counter ticks)
    TIM2->ARR = 1000 - 1;                      // 3. auto-reload: period = (ARR + 1) = 1000 ticks = 1 ms -> 1000 periods per second = 1 kHz (how far it counts before wrapping to 0)
                                               //    Frequency (or period): how often the on/off cycle repeats. Here 1 kHz, so one cycle every 1 ms.
    TIM2->CCR1 = 250;                          // 4. compare: duty = CCR1 / (ARR + 1) = 250 / 1000 = 25 %. Where in the lap the output flips:
                                               //    high for ticks 0–249, low for 250–999. Duty cycle = the fraction of each cycle that's high:
                                               //    25 %, so 250 µs high, 750 µs low.

    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M_Msk;        // 5. clear output-compare mode bits for channel 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // set PWM mode 1 (OC1M = 110): output high while CNT < CCR1, low otherwise
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;            // enable preload for channel 1 (preloads CCR1, which is the duty cycle)

    TIM2->CCER |= TIM_CCER_CC1E;               // 6. enable output for channel 1. CCER is the on/off switch: it says whether the channel's result
                                               //    actually reaches the pin. CC1E = 1 connects channel 1's internal signal (OC1) to the output.

    TIM2->CR1 |= TIM_CR1_ARPE;                 // 7. enable ARR preload (same idea as OC1PE, applied to the period)
    TIM2->EGR |= TIM_EGR_UG;                   // 8. generate an update event to load the registers that have a waiting room: PSC, ARR and CCR1 (PSC's waiting room is always on)
                                               //    Preload means "new values take effect at the next wrap." UG makes an artificial wrap since the timer hasn't started yet:
                                               //    it copies the waiting-room values into the active registers and resets CNT to 0.

    TIM2->CR1 |= TIM_CR1_CEN;                  // 9. start the counter: counts up from 0 to ARR, wraps back to 0, repeats, with no CPU involvement

}

// we need a delay so the duty cycle change is visible to the eye. 
//The timer is running at 1 kHz, so each period is 1 ms. 
//The LED will be on for 25 % of that period, or 250 µs, and off for 750 µs. 
//If we change the duty cycle every 5 ms, the eye will see a smooth fade in and out.
// we assume each iteration costs 10 cpu cycles so we would need 8000 iterations to get a 5 ms delay at 16 MHz.
static void delay_rough(uint32_t loops)
{
    for (volatile uint32_t i = 0; i < loops; i++) { }
}




int main(void)
{
    volatile uint32_t r = cpp_selftest();
    (void)r;   // tells the compiler "I'm keeping r on purpose even if unused"

    gpio_init();  // PA5 -> TIM2_CH1 (AF1)
    pwm_init();   // TIM2 CH1: 1 kHz, 25 % duty

    uint32_t duty = 0;
    int32_t  step = 10;  // how much to change the duty cycle each time through the loop 10 counts is 1 % of the 1000-count period

    while (1) {
        /* nothing: the timer runs on its own */
        TIM2->CCR1 = duty;
        duty += step;
        if (duty >= 1000u || duty == 0u) step = -step; // reverse direction at the ends of the range
        delay_rough(8000);                   /* ~5 ms at 16 MHz, -O0 */ 
    }



    /* Step 2 blink loop, kept for reference:
    while (1) {
        // XOR so that if the bit is 0 it becomes 1, and if it is 1 it becomes 0.
        GPIOA->ODR ^= GPIO_ODR_OD5;           // toggle PA5 (break here to watch ODR before and after)
        delay();                              // wait (delay loop)
    }
    */
}