#include "custom_timer.h"
#include <timers.h>

void initTMR0() {
    T08BIT = 0; // 8bit mode
    T0CS = 0; // Internal clock (Fosc/4))
    T0SE = 0; // Increment on low-to-high transition
    PSA = 1; // No prescaler assigned
    //T0PS = 0b000; // prescale 1:2 (not used due to previous line)
}

inline void startTMR0For1kHz() {
    TMR0H = 0xF0;
    TMR0L = 0x60;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1; 
    TMR0ON = 1;
}

inline void stopTMR0() {
    TMR0ON = 0;
    INTCONbits.TMR0IE = 0;
}

// TIMER0: f_out = 1kHz, interrupt every 1ms 
//    // Reload for TMR0H:TMR0L : 61536
//    TMR0H = 0xF0;
//    TMR0L = 0x60; // counter preload
//    T08BIT = 0; // 16bit mode
//    T0CS = 0; // Internal clock (Fosc/4))
//    T0SE = 0; // Increment on low-to-high transition
//    PSA = 1; // No prescaler assigned
//    TMR0ON = 1; // enables timer0
//    TMR0IE = 1; // enables timer0 interrupts