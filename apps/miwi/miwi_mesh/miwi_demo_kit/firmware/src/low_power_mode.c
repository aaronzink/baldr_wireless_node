/*
 * File:   low_power_mode.c
 * Author: Luke
 *
 * Created on January 25, 2017, 12:33 PM
 */


#include "low_power_mode.h"
#include "system.h"

void wait(void) {
    //For Sleep
    //(assign the ULPOUT signal in the PPS module to a pin
    //which has also been assigned an interrupt capability,
    //such as INT1)
    INTCON3bits.INT1IF = 0;
    INTCON3bits.INT1IE = 1;
    //********************
    //Configure Sleep Mode
    //********************
    //For Sleep
    OSCCONbits.IDLEN = 0;
    //****************
    //Enter Sleep Mode
    //****************
    Sleep();
}

void power_down(void) {
    int i = 0;
    
    //Configure a remappable output pin with interrupt capability
    //for ULPWU function (RP21 => RD4/INT1 in this example)
    //*********************************************************************************
    RPOR21 = 13;// ULPWU function mapped to RP21/RD4
    RPINR1 = 21;// INT1 mapped to RP21 (RD4)
    //***************************
    //Charge the capacitor on RA0
    //***************************
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 1;
    //TODO: determine how long we should charge the capacitor for
    for(i = 0; i < 10000; i++) Nop();
    //**********************************
    //Stop Charging the capacitor on RA0
    //**********************************
    TRISAbits.TRISA0 = 1;
    //*****************************************
    //Enable the Ultra Low Power Wakeup module
    //and allow capacitor discharge
    //*****************************************
    WDTCONbits.ULPEN = 1;
    WDTCONbits.ULPSINK = 1;
    //******************************************
    //Enable Interrupt for ULPW
    //******************************************
    //For Sleep
    //(assign the ULPOUT signal in the PPS module to a pin
    //which has also been assigned an interrupt capability,
    //such as INT1)
    INTCON3bits.INT1IF = 0;
    INTCON3bits.INT1IE = 1;
    //********************
    //Configure Sleep Mode
    //********************
    //For Sleep
    OSCCONbits.IDLEN = 0;
    //For Deep Sleep
    OSCCONbits.IDLEN = 0; // enable deep sleep
    DSCONHbits.DSEN = 1; // Note: must be set just before executing Sleep();
    //****************
    //Enter Sleep Mode
    //****************
    Sleep();
    // for sleep, execution will resume here
    // for deep sleep, execution will restart at reset vector (use WDTCONbits.DS to detect)
}

void deep_sleep(void)
{
    // Disable IDLE mode on Sleep() instruction
    OSCCONbits.IDLEN = 0;

    // Enable wake using MCLR
    DSWAKELbits.DSMCLR = 1;
    DSWAKELbits.DSMCLR = 1; // twice

    // Enable Wake using DS watchdog timer
    DSWAKELbits.DSWDT = 1;
    DSWAKELbits.DSWDT = 1; // twice
    
    // Enable wake using ULP wake-up
    //DSWAKELbits.DSULP = 1;

    // Note: writing DSEN then executing sleep must happen back-to-back for deep sleep to work.
    // See DS30575A page 77
    DSCONL = 0;
    DSCONL = 0; // twice
    DSCONH = 0;
    DSCONH = 0; // twice
    DSCONHbits.DSEN = 1;
    DSCONHbits.DSEN = 1; // twice
    Sleep();
}
