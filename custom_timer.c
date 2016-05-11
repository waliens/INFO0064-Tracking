#include "custom_timer.h"
#include <timers.h>

void initTMR0For1kHz() {
    resetTMR0For1kHz();
    T08BIT = 0; // 8bit mode
    T0CS = 0; // Internal clock (Fosc/4))
    T0SE = 0; // Increment on low-to-high transition
    PSA = 1; // No prescaler assigned
    //T0PS = 0b000; // prescale 1:2 (not used due to previous line)
    TMR0ON = 1; // enables timer0
    TMR0IE = 1; // enables timer0 interrupts
}

inline void resetTMR0For1kHz() {
    TMR0H = 0xF0;
    TMR0L = 0x60;
}

inline void stopTMR0() {
    TMR0ON = 0;
}