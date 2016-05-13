/*
 * File:   main.c
 *
 * Created on August 16, 2010, 12:09 PM
 */

#include "p18f24k50.h"
#include <xc.h>
#include "custom_timer.h"
#include "protocol.h"
#include <timers.h>
#include <stdbool.h>
#include <stdio.h>
 
#pragma config FOSC = INTOSCIO
#pragma config MCLRE = ON
#pragma config PBADEN = OFF
#pragma config LVP = OFF
#pragma config WDTEN = OFF, DEBUG = OFF

#define PING_RECEIVED_LOW 464
#define PING_RECEIVED_HIGH 564

#define MODE_RECV 1 
#define MODE_WAITING_BEFORE_SEND 2
#define MODE_SENDING 3
#define MODE_WAITING_AFTER_SEND 4

#define MODE_WAIT1_DURATION 15
#define MODE_SEND_DURATION 16
#define MODE_WAIT2_DURATION 31

#define LED_PORT1 LATAbits.LATA0
#define LED_PORT2 LATAbits.LATA1

/**
 * Mode of the program
 */
volatile char mode = MODE_RECV;

volatile int tmr0_counter = 0;


static inline void initADCON();
static inline void initLedPorts();
static inline void initOscillator();
static inline void initPWM();
static inline void startPWM();
static inline void stopPWM();
static inline void startADC();

//interrupt vector
void interrupt interrupt_service_routine(void) {    
    // ADC conversion finished
    if (PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0; // reset end of conversion flag
        int val = (ADRESH << 2) | (ADRESL >> 6);
        if (val < PING_RECEIVED_LOW || val > PING_RECEIVED_HIGH) {
            LED_PORT1 = 0;
            mode = MODE_WAITING_BEFORE_SEND;
            tmr0_counter = 0;
            startTMR0For1kHz();
        } else {
            startADC();
        }
        return;
    }
    
    // Timer0 overflow
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF) { 
        INTCONbits.TMR0IF = 0;
        switch(mode) {
            case MODE_WAITING_BEFORE_SEND:
                if (tmr0_counter >= MODE_WAIT1_DURATION) {
                    mode = MODE_SENDING;
                    startPWM();
                }
                break;
            case MODE_SENDING: 
                if (tmr0_counter >= MODE_SEND_DURATION) {
                    stopPWM();
                    mode = MODE_WAITING_AFTER_SEND;
                }
                break;
            case MODE_WAITING_AFTER_SEND:
                if (tmr0_counter >= MODE_WAIT2_DURATION) {
                    stopTMR0();
                    mode = MODE_RECV;
                    startADC();
                    LED_PORT1 = 1;
                }
        }
        LED_PORT2 = !LED_PORT2;
        tmr0_counter++;
        preloadTMR0For1kHz();
        return;
    }
    
}

/**
 * Init the ADC to read values from RB3
 */
static inline void initADCON() {
    // Init ADCON0
    ADCON0bits.ADON = 1; // Enable ADC module
    ADCON0bits.GO_nDONE = 0; // Reset GO to 0
    ADCON0bits.CHS = 0b01001; // Use RB3 as input channel
    ANSELBbits.ANSB3 = 1; // Set RB3 as analog input
    TRISBbits.TRISB3 = 1; // Set RB3 pin as input
    // Init ADCON1
    ADCON1bits.TRIGSEL = 0; // special trigger from CCP2
    ADCON1bits.PVCFG = 0; // connect reference Vref+ to internal Vdd
    ADCON1bits.NVCFG = 0; // connect reference Vref- to external Vdd
    // Init ADCON2
    ADCON2bits.ADFM = 0; // result format is left justified
    ADCON2bits.ACQT = 0b100; // 8 TAD
    ADCON2bits.ADCS = 0b101; // Fosc / 16
    // Enable ADC interrupts
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
}

/**
 * Initialize RA0 to be a digital output 
 */
static inline void initLedPorts() {
    TRISAbits.TRISA0 = 0; // Port RA0 = output
    LATAbits.LATA0 = 0; // Clear RA0 output (set 0V)
    TRISAbits.TRISA1 = 0; // Port RA1 = output 
    LATAbits.LATA1 = 0; // Clear RA1 output (set 0V)
}

/**
 * Init oscillator to be internal and 16MHz
 */
static inline void initOscillator() {
    OSCCONbits.IDLEN = 1; // Device enters in sleep mode
    OSCCONbits.IRCF = 0b111; // Internal oscillator set to 16MHz
    OSCCONbits.OSTS = 0; // Running from internal oscillator
    OSCCONbits.HFIOFS = 0; // Frequency not stable
    OSCCONbits.SCS = 0b00; // Primary clock defined by FOSC<3:0>
}

static inline void startADC() {
    ADCON0bits.GO = 1;
}

static inline void initPWM() {
    TRISC1 = 0;  // set PORTC as output, RC1 is the pwm pin output
    PORTC = 0;   // clear PORTC
    PR2 = 0b00011000 ; // 24 : PWM Period value
    CCP2CON = 0b00011100; // 00 (unused) : 01(These bits are the two LSbs of the PWM duty cycle. The eight MSbs are found in CCPRxL.) : 1100 (PWM mode))
    CCPR2L = 0b00001100; // -> 0b0000110010 = 50% duty cycle

    T2CKPS1 = 0;    // Prescaler value - High bit
    T2CKPS0 = 1;    // Prescaler value - Low bit
}
static inline void startPWM() {
    TRISC1 = 0;
    TMR2ON = 1;  
}

static inline void stopPWM() {
    // Stop PWM
    TMR2ON = 0;
    TRISC1 = 1;
}


void main(void){
    initOscillator();
    initTMR0();
    initLedPorts();
    initADCON();
    initUART();
    initPWM();

    // Mode of the pic
    mode = MODE_RECV;
    // enable interrupts
    GIEH = 1;
    PEIE = 1;
    
    startADC();
    LED_PORT1 = 1;
    
    while(1) { } 
}
