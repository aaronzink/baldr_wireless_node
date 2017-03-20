/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/
#include "system.h"
#include "system_config.h"
#include "miwi/miwi_api.h"


// Demo Version
#define MAJOR_REV       1
#define MINOR_REV       3

//use to activate LCD
#define DEBUG_LCD 1
//use to activate LEDs
#define DEBUG_LED 1

/*************************************************************************/
// The variable myChannel defines the channel that the device
// is operate on. This variable will be only effective if energy scan
// (ENABLE_ED_SCAN) is not turned on. Once the energy scan is turned
// on, the operating channel will be one of the channels available with
// least amount of energy (or noise).
/*************************************************************************/

// Possible channel numbers are from 0 to 31
uint8_t myChannel = 26;

#define MiWi_CHANNEL        0x04000000                //Channel 26 bitmap

//previously packets were assigned up to 5, start at 6
#define ALBATROSS_ACK       6
#define ALBATROSS_ALERT     7
#define ALBATROSS_NO_ALERT  8
#define ALBATROSS_INIT      9
#define ALBATROSS_CONNECT   10

uint8_t ConnectionEntry = 0;

volatile bool alert = false;

extern uint8_t myLongAddress[MY_ADDRESS_LENGTH];

/*************************************************************************/
// AdditionalNodeID variable array defines the additional 
// information to identify a device on a PAN. This array
// will be transmitted when initiate the connection between 
// the two devices. This  variable array will be stored in 
// the Connection Entry structure of the partner device. The 
// size of this array is ADDITIONAL_NODE_ID_SIZE, defined in 
// ConfigApp.h.
// In this demo, this variable array is set to be empty.
/*************************************************************************/
#if ADDITIONAL_NODE_ID_SIZE > 0
    uint8_t AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE] = {0x00};
#endif

//TODO: this could be useful for persisting data through sleep / power off
//void memoryTest()
//{
//    uint8_t testValue[5] = {1,2,3,4,5};
//    uint8_t address = 5000;
//    uint8_t count = (uint8_t)sizeof(testValue);
//    uint8_t * testOutput[5];
//    
//    SSTWrite(&testValue, address, count);
//    SSTRead(&testOutput, address, count);
//    
//#if DEBUG_LCD
//    LCD_Erase();
//    sprintf((char *)LCDText, "%03d,%03d,%03d,%03d",testOutput[0],testOutput[1],testOutput[2],testOutput[3]);
//    sprintf((char *)&(LCDText[16]), (char*)"Memory Test     ");
//    LCD_Update();
//#endif
//    
//    DELAY_ms(10000);
//    
//}

void set_alert(bool val)
{
    if(val){
        //disable the INT0 interrupt since we are already alerted
        INTCONbits.INT0IE = 0;

        alert = true;
#if DEBUG_LED
        LED1 = 1;
#endif
    }
    else
    {
        alert = false;
#if DEBUG_LED
        LED1 = 0;
#endif
        
        //enable the INT0 interrupt for the next alert
        INTCONbits.INT0IE = 1;
    }
}
    
void setup_transceiver(bool restore)
{
    Read_MAC_Address();
    
    //P2P_TRANSMISSION
    
    //CreateNewConnection(1);

    /*******************************************************************/
    // Initialize the MiWi Protocol Stack. The only input parameter indicates
    // if previous network configuration should be restored.
    /*******************************************************************/
    MiApp_ProtocolInit(restore);

    if( MiApp_SetChannel(myChannel) == false )
    {
#if DEBUG_LCD
        LCD_Display((char *)"ERROR: Unable toSet Channel..", 0, true);
#endif
#if DEBUG_LED
        LED0 = 0;
        LED2 = 1;
#endif
        //TODO: should we try something else here? Maybe notify the user of the error?

        //we couldn't set the channel so sleep and then try again
        enter_deep_sleep();
    }
    
    /*******************************************************************/
    //  Set the connection mode. The possible connection modes are:
    //      ENABLE_ALL_CONN:    Enable all kinds of connection
    //      ENABLE_PREV_CONN:   Only allow connection already exists in 
    //                          connection table
    //      ENABL_ACTIVE_SCAN_RSP:  Allow response to Active scan
    //      DISABLE_ALL_CONN:   Disable all connections. 
    /*******************************************************************/
    MiApp_ConnectionMode(ENABLE_ALL_CONN);
}

/*******************************************************************/
// Perform an active scan
/*******************************************************************/
uint8_t scan_for_network()
{
    volatile uint8_t scanresult;
    
    if(myChannel < 8)
        scanresult = MiApp_SearchConnection(10, (0x00000001 << myChannel));
    else if(myChannel < 16)
        scanresult = MiApp_SearchConnection(10, (0x00000100 << (myChannel-8)));
    else if(myChannel < 24)
        scanresult = MiApp_SearchConnection(10, (0x00010000 << (myChannel-16)));
    else
        scanresult = MiApp_SearchConnection(10, (0x01000000 << (myChannel-24)));

    return scanresult;
}

void connect_to_network(bool first_time)
{
    uint8_t scanresult;

#if DEBUG_LCD
    LCD_Display((char *)"  Scanning for    Networks....", 0, false);
#endif
    
    uint8_t j, k;
    bool connected = false;
    while(!connected)
    {
        scanresult = scan_for_network();

        /*******************************************************************/
        // Display Scan Results
        /*******************************************************************/
        if(scanresult > 0)
        {   
            for(j = 0; j < scanresult; j++)
            {
                uint8_t skip_print = false;
                if(j > 0)
                {
                    for(k = 0; k < j; k++)
                    {
                        if((ActiveScanResults[j].PANID.v[1] == ActiveScanResults[k].PANID.v[1]) &
                            (ActiveScanResults[j].PANID.v[0] == ActiveScanResults[k].PANID.v[0]))
                        {
                            skip_print = true;
                            break;
                        }

                    }

                    if(skip_print == true)
                    { 
                        if(j == (scanresult-1)) j = -1;
                        continue;
                    }
                }

                /*******************************************************************/
                // Connect to Display Network
                /*******************************************************************/
                uint8_t Status;
                uint8_t CoordCount = 0;
                MiApp_FlushTx();

                for(k = 0 ; k < scanresult ; k++)
                {
                    if((ActiveScanResults[j].PANID.v[1] == ActiveScanResults[k].PANID.v[1]) &
                        (ActiveScanResults[j].PANID.v[0] == ActiveScanResults[k].PANID.v[0]))
                    {
                        CoordCount++;
                    }
                }
                if(CoordCount > 1)
                {
                    MiApp_FlushTx();
                    if(first_time)
                    {
                       MiApp_WriteData(ALBATROSS_INIT);
                    }
                    else
                    {
                       MiApp_WriteData(ALBATROSS_CONNECT);
                    }
                    MiApp_WriteData(ActiveScanResults[j].PANID.v[1]);
                    MiApp_WriteData(ActiveScanResults[j].PANID.v[0]);
                    MiApp_BroadcastPacket(false);
                    for(k = 0; k < scanresult; k++)
                    {
                        if((ActiveScanResults[j].PANID.v[1] == ActiveScanResults[k].PANID.v[1]) &
                            (ActiveScanResults[j].PANID.v[0] == ActiveScanResults[k].PANID.v[0]))
                        {
                            //Establish connection with the node
                            j = k;
                            k = scanresult-1;
                            break;
                        }
                    } //End of for(k = 0; ....)
                } //End of if (CoordCount > ...)

                /*******************************************************************/
                // Establish Connection and Display results of connection
                /*******************************************************************/
                Status = MiApp_EstablishConnection(j, CONN_MODE_DIRECT);
                if(Status == 0xFF)
                {
#if DEBUG_LCD
                    LCD_Display((char *)"Join Failed!!!", 0, true);
#endif
#if DEBUG_LED
                    LED2 = 1;
#endif
                    connected = false;
                    //TODO: this currently just loops and tries again, should we do something more intelligent?
                }
                else
                {
                    MiApp_FlushTx();
                    MiApp_WriteData(ALBATROSS_ACK);
                    //TODO: build fix
//                    MiApp_WriteData(myPANID.v[1]);
//                    MiApp_WriteData(myPANID.v[0]);
                    MiApp_BroadcastPacket(false);
                    
#if DEBUG_LCD
                    LCD_Display((char *)"Joined  Network Successfully..", 0, true);
#endif
#if DEBUG_LED
                    LED2 = 0;
#endif

                    connected = true;
                    break;
                }
            }
        }else
        {
            //TODO: there is no network, what do we do?
            connected = false;
        }
    }
    
#if DEBUG_LCD
    LCD_Erase();
    //TODO: build fix
//    sprintf((char *)(LCDText), (char*)"PANID:%02x%02x Ch:%02d",myPANID.v[1],myPANID.v[0],myChannel);
//    sprintf((char *)&(LCDText[16]), (char*)"Address: %02x%02x", myShortAddress.v[1], myShortAddress.v[0]);
    LCD_Update();
    
    DELAY_ms(1000);
#endif
    
    //make sure there are no lingering messages
    MiApp_DiscardMessage();
}

bool check_acknowledge()
{
    bool acknowledged = false;
    uint8_t pktCMD = 0;
    //TODO: should we check for multiple messages?
    if(MiApp_MessageAvailable())
    {
        pktCMD = rxMessage.Payload[0];
        MiApp_DiscardMessage();
        if(pktCMD == ALBATROSS_ACK)
        {
            acknowledged = true;
#if DEBUG_LCD
            LCD_Display((char *)"Received        Acknowledge", 0, true);
#endif
        }
        else //unknown packet
        {
            acknowledged = false;
#if DEBUG_LCD
            LCD_Display((char *)"Received UnknownPacket! %02d", pktCMD, true);
#endif
        }
#if DEBUG_LCD
        LCD_Erase();
#endif
    }
    
    return acknowledged;
}

void send_message()
{
    bool send = true;
    while(send) {
        if(alert)
        {
            MiApp_FlushTx();
            MiApp_WriteData(ALBATROSS_ALERT);
            MiApp_BroadcastPacket(false);
#if DEBUG_LCD
            LCD_Display((char *)"Sending ALBATROSS      Alert    ", 0, false);
#endif
        }
        else
        {
            MiApp_FlushTx();
            MiApp_WriteData(ALBATROSS_NO_ALERT);
            MiApp_BroadcastPacket(false);
#if DEBUG_LCD
            LCD_Display((char *)"Sending ALBATROSS      No Alert ", 0, false);
#endif
        }
    
        if(check_acknowledge())
        {
            send = false;
        }
    }

    set_alert(false);
}

/*********************************************************************
* Function:         void main(void)
*
* PreCondition:     none
*
* Input:		    none
*
* Output:		    none
*
* Side Effects:	    none
*
* Overview:		    This is the main function that runs the simple 
*                   example demo. The purpose of this example is to
*                   demonstrate the simple application programming
*                   interface for the MiWi(TM) Development 
*                   Environment. By virtually total of less than 30 
*                   lines of code, we can develop a complete 
*                   application using MiApp interface. The 
*                   application will first try to establish a P2P 
*                   link with another device and then process the 
*                   received information as well as transmit its own 
*                   information.
*                   MiWi(TM) DE also support a set of rich 
*                   features. Example code FeatureExample will
*                   demonstrate how to implement the rich features 
*                   through MiApp programming interfaces.
*
* Note:			    
**********************************************************************/
void main(void)
{
    /*******************************************************************
     *LED0 = running / sleeping
     *LED1 = alert / no alert
     *LED2 = error / no error
     *All LEDs = first time setup
    *******************************************************************/
    
    
    bool ds_wake = false;

    if (WDTCONbits.DS)   // Woke up from deep sleep
    {
        DSCONLbits.RELEASE = 0;    // release control and data bits for all I/O ports
        WDTCONbits.DS = 0;       // clear the deep-sleep status bit
        ds_wake = true;
    }
    
    /*******************************************************************/
    // Initialize Hardware
    /*******************************************************************/
    SYSTEM_Initialize();
    
#if DEBUG_LCD
    LCD_Initialize();
#endif

#if DEBUG_LED
    LED0 = 1;
    LED1 = LED2 = 0;
#endif
    
    /*******************************************************************/
    // Display Start-up Splash Screen
    /*******************************************************************/
#if DEBUG_LCD
    if(ds_wake)
    {
        LCD_BacklightON();
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"    Wake        ");
        
        if (DSWAKELbits.DSMCLR)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  MCLR          ");
        }
        else if (DSWAKELbits.DSWDT)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  DSWDT         ");
        }
        else if (DSWAKEHbits.DSINT0)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  INT0          ");
        }
        else if(DSWAKELbits.DSULP)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  ULPWU         ");
        }
        
        LCD_Update();
        DELAY_ms(2000);
        LCD_BacklightOFF();
    }
    else
    {
        LCD_BacklightON();
        
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"Albatross       ");
        sprintf((char *)&(LCDText[16]), (char*)"Sensor Board    ");
        LCD_Update();
        DELAY_ms(2000);
        LCD_BacklightOFF();
    }
#endif
    
    do
    {
        if(ds_wake)
        {
            //we woke because a sensor was tripped, notify of intrusion
            if(DSWAKEHbits.DSINT0)
            {
                set_alert(true);
            }

            setup_transceiver(false);
            connect_to_network(false);
            send_message();
        }
        else //first power on
        {
#if DEBUG_LED
            LED0 = LED1 = LED2 = 1;
#endif
            setup_transceiver(false);
            connect_to_network(true);

#if DEBUG_LCD
            LCD_Display((char *)"Connected to    main hub     ", 0, true);
#endif
        }

        //put the transceiver to sleep, very important for power saving
        MiApp_TransceiverPowerState(POWER_STATE_SLEEP);
    } while (alert); //make sure there is no longer an alert

#if DEBUG_LCD
    LCD_Erase();
#endif
    
#if DEBUG_LED
    LED0 = LED1 = LED2 = 0;
#endif

    enter_deep_sleep();
}

// ISR
void UserInterruptHandler(void)
{
    //check for INT0 interrupt
    if(INTCONbits.INT0IF == 1)
    {
        //TODO: this is correct
        if(AUX1_PORT || AUX2_PORT)
        {
            set_alert(true);
        }
        
        //clear the interrupt
        INTCONbits.INT0IF = 0;
    }
}
