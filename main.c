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

#define MODE_UART 0
#define MODE_RECV 1 

#define TMR2_PR_32KHZ 127

#define LED_PORT LATAbits.LATA0

/**
 * Mode of the program
 */
volatile char mode = MODE_RECV;
volatile int counter = 0;
/**
 * ADC fields
 */
volatile int adc_val = -1;

//interrupt vector
void interrupt interrupt_service_routine(void) {
    // Timer2 overflow
    if (TMR2IE && TMR2IF) { 
        TMR2IF = 0;
        ADCON0bits.GO = 1;
    }
    
    // ADC conversion finished
    if (PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0; // reset end of conversion flag
        adc_val = (ADRESH << 2) | (ADRESL >> 6);
    }
}

/**
 * Init the ADC to read values from RB3
 */
static void initADCON() {
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
static void initLedOnRA0() {
    TRISAbits.TRISA0 = 0; // Port RA0 = output
    LATAbits.LATA0 = 0; // Clear RA0 outputs (set 0V)
}

/**
 * Init oscillator to be internal and 16MHz
 */
static void initOscillator() {
    OSCCONbits.IDLEN = 1; // Device enters in sleep mode
    OSCCONbits.IRCF = 0b111; // Internal oscillator set to 16MHz
    OSCCONbits.OSTS = 0; // Running from internal oscillator
    OSCCONbits.HFIOFS = 0; // Frequency not stable
    OSCCONbits.SCS = 0b00; // Primary clock defined by FOSC<3:0>
}

void startTMR2() {
    // TIMER2: f_out = ~30kHz
    // Period : 127
    PR2 = 191;
    OpenTimer2(TIMER_INT_ON 
                & T2_PS_1_1
                & T2_POST_1_1);
}

void main(void){
    initOscillator();
    initLedOnRA0();
    initADCON();
    initUART();

    // Mode of the pic
    mode = MODE_RECV;
    startTMR2();
    
    // enable interrupts
    GIEH = 1;
    PEIE = 1;
    //int i = 0;
    while(1) {
        switch(mode) {
            case MODE_UART:
                LED_PORT = 1; // light up led
                send_debug("Ping received.");
                for (int i = 0; i < 20000; ++i);
                mode = MODE_RECV;
                TMR2ON = 1;
                LED_PORT = 0; // shut off led
                break;
            case MODE_RECV:
                // check thresholds
                PIE1bits.ADIE = 0;
                int val = adc_val;
                PIE1bits.ADIE = 1;
               
                if (val < PING_RECEIVED_LOW || val > PING_RECEIVED_HIGH) {
                    TMR2ON = 0;
                    mode = MODE_UART;
                } 
                break;
        }
    } 
}
