/*
 * File:   low_power_mode.c
 * Author: Luke
 *
 * Created on January 25, 2017, 12:33 PM
 */


#include "low_power_mode.h"
#include "system.h"

void enter_idle(void) {
    //enable IDLE
    OSCCONbits.IDLEN = 1;
    
    //Enter IDLE Mode
    Sleep();
    NOP();
}

void enter_sleep(void) {
    //enable Sleep (disable IDLE)
    OSCCONbits.IDLEN = 0;
    
    //Enter Sleep Mode
    Sleep();
    NOP();
}

void enter_deep_sleep(void) {
    //disable global interrupts to prevent interrupts before sleeping
    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;

    //For Deep Sleep
    OSCCONbits.IDLEN = 0; // enable sleep
    DSCONHbits.DSEN = 1; // Note: must be set immediately before executing Sleep();
    //Enter deep sleep Mode
    Sleep();
    NOP();
    // execution will restart at reset vector (start of main()) (use WDTCONbits.DS to detect)
}

//TODO: ULP doesn't work because there is no capacitor connected to RA0
void enter_deep_sleep_ulp(void) {
    //TODO: move these to the PPS configuration section in system.c to use ULP
    //Configure a remappable output pin with interrupt capability for ULPWU function
    //RPOR6 = 13;// ULPWU function (13) mapped to RP6/RD4
    //RPINR2 = 6;// INT2 mapped to RP6 (RB3)
    
    
    int i = 0;

    //Charge the capacitor on RA0
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 1;
    //TODO: determine how long we should charge the capacitor for, I think there is an on board calibration mechanism
    for(i = 0; i < 10000; i++) Nop();
    //Stop Charging the capacitor on RA0
    TRISAbits.TRISA0 = 1;
    
    //Enable the Ultra Low Power Wakeup module
    DSCONHbits.DSULPEN = 1;
    DSCONLbits.ULPWDIS = 0;
    
    //Enable the Ultra Low Power Wakeup module
    //and allow capacitor discharge
    //TODO: do we need to enable ULP from WDT as well as DS?
    //WDTCONbits.ULPEN = 1;
    WDTCONbits.ULPSINK = 1;
    //Enable Interrupt for ULPW
    INTCON3bits.INT2IF = 0;
    INTCON3bits.INT2IE = 1;
    
    //TODO: can we disable global interrupts and still use ULPWU?
    //disable global interrupts to prevent interrupts before sleeping
//    INTCONbits.GIEH = 0;
//    INTCONbits.GIEL = 0;

    //For Deep Sleep
    OSCCONbits.IDLEN = 0; // enable sleep
    DSCONHbits.DSEN = 1; // Note: must be set immediately before executing Sleep();
    //Enter deep sleep Mode
    Sleep();
    NOP();
    // execution will restart at reset vector (start of main()) (use WDTCONbits.DS to detect)
}
