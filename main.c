/*
 * File:   main.c
 *
 * Created on August 16, 2010, 12:09 PM
 */

#include "p18f24k50.h"
#include <xc.h>
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

volatile int result = 0;
volatile bool ping_received = false;

static void initADCON() {
    // Init ADCON0
    ADCON0bits.ADON = 1; // Enable ADC module
    ADCON0bits.GO_nDONE = 0; // Reset GO to 0
    ADCON0bits.CHS = 0b00001; // Use RA1 as input channel
    TRISAbits.TRISA1 = 1; // Set RA1 pin as input
    ANSELAbits.ANSA1 = 1; // Set RA1 as analog input
    // Init ADCON1
    ADCON1bits.TRIGSEL = 0; // special trigger from CCP2
    ADCON1bits.PVCFG = 0; // connect reference Vref+ to internal Vdd
    ADCON1bits.NVCFG = 0; // connect reference Vref- to external Vdd
    // Init ADCON2
    ADCON2bits.ADFM = 0; // result format is left justified
    ADCON2bits.ACQT = 0b100; // 8 TAD
    ADCON2bits.ADCS = 0b101; // Fosc / 16
}

static void initOscillator() {
    OSCCONbits.IDLEN = 1; // Device enters in sleep mode
    OSCCONbits.IRCF = 0b111; // Internal oscillator set to 16MHz
    OSCCONbits.OSTS = 0; // Running from internal oscillator
    OSCCONbits.HFIOFS = 0; // Frequency not stable
    OSCCONbits.SCS = 0b00; // Primary clock defined by FOSC<3:0>
}

static void initPortB() {
    TRISB = 0; // PORTB = output
    LATB = 1; // Clear B outputs (set 0V)
    // Turn off the LED outputs
    LATBbits.LATB3 = 0;
    LATBbits.LATB4 = 0;
    LATBbits.LATB5 = 0;
}

static void outputPWM() {
    TRISC1 = 0;  // set PORTC as output, RC1 is the pwm pin output
    PORTC = 0;   // clear PORTC
    //PR2 = 0b01100011;
    
    /* PWM registers configuration
    * Fosc = 16000000 Hz
    * Fpwm = 40000.00 Hz (Requested : 40000 Hz)
    * Duty Cycle = 50 %
    * Resolution is 8 bits
    * Prescaler is 4
    * Ensure that your PWM pin is configured as digital output
    * see more details on http://www.micro-examples.com/
    * this source code is provided 'as is',
    * use it at your own risks
    */
    PR2 = 0b00011000 ;
    CCP2CON = 0b00011100;
    CCPR2L = 0b00001100;
    T2CKPS1 = 0;    // Prescaler value - High bit
    T2CKPS0 = 1;    // Prescaler value - Low bit
    TMR2ON = 1;     // Activate timer 2
  
}

static inline void enableGlobalInterrupts() {
    GIEH = 1;
}

static inline void enablePeripheralInterrupts() {
    PEIE = 1;
}

static inline void resetTMR2To200kHz() {
    TMR2 = 236;
}

static inline void resetTMR0To1kHz() {
    TMR0H = 0xF0;
    TMR0L = 0x60;
}

//interrupt vector
void interrupt MyIntVec(void) {
    // Timer0 interrup 
    if (TMR0IE == 1 && TMR0IF == 1) {
        TMR0IF = 0;
        ADCON0bits.GO = 1;
    }
    
    // ADC conversion finished
    if (PIE1bits.ADIE == 1 && PIR1bits.ADIF == 1 ) {
        PIR1bits.ADIF = 0; // reset end of conversion flag
        int adc_input = (ADRESH << 2) | (ADRESL >> 6);
        if (adc_input < PING_RECEIVED_LOW || adc_input > PING_RECEIVED_HIGH) {
            ping_received = true;
        }
    }
}

void main(void){
    initOscillator();
    initPortB();
    
    // Timer0: f_out: 1kHz
    // Reload for TMR0H:TMR0L: 61536
    resetTMR0To1kHz();
    OpenTimer0(TIMER_INT_ON 
                & T0_16BIT
                & T0_SOURCE_INT
                & T0_PS_1_1);
    
    // Timer2: f_out: 200kHz (which is Fin / 20 where Fin = Fosc / 4)
    // Reload for TMR2: 236
    resetTMR2To200kHz();
    OpenTimer2(TIMER_INT_ON 
                & T2_PS_1_1
                & T2_POST_1_1);
    
    
    // enable interrupts
    GIEH = 1;
    PEIE = 1;
    
    LATB3 = 0; 

    int i = 0;
    while(1) {
        if (i % 50000 == 0) {
            LATB3 = !LATB3;
        }
        i++;
    }
}
