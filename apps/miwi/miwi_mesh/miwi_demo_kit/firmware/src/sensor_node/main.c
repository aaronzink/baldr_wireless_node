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
#include "self_test.h"
#include "range_demo.h"
#include "temp_demo.h"


// Demo Version
#define MAJOR_REV       1
#define MINOR_REV       3

/*************************************************************************/
// The variable myChannel defines the channel that the device
// is operate on. This variable will be only effective if energy scan
// (ENABLE_ED_SCAN) is not turned on. Once the energy scan is turned
// on, the operating channel will be one of the channels available with
// least amount of energy (or noise).
/*************************************************************************/

// Possible channel numbers are from 0 to 31
uint8_t myChannel = 26;

//use to activate LEDs, LCD, button inputs
#define DEBUG 1

#define MiWi_CHANNEL        0x04000000                //Channel 26 bitmap

#define EXIT_DEMO           1
#define RANGE_DEMO          2
#define SECURITY_DEMO       3
#define IDENTIFY_MODE       4
#define EXIT_IDENTIFY_MODE  5
#define DEMO_ALERT          6
#define DEMO_NO_ALERT       7
#define DEMO_ACK            8

#define NODE_INFO_INTERVAL  5

uint8_t ConnectionEntry = 0;
			
bool NetFreezerEnable = false;
bool SleepTest = false;
bool PowerTest = false;
bool memTest = false;
bool AutoConnectNetwork = false; //Create or join network on channel 26
bool AutoStartDemo = false; //start the security_demo() automatically)

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
    
void TestSleep(void)
{   
    bool ds_wake = false;

    if (WDTCONbits.DS)   // Woke up from deep sleep
    {
        ds_wake = true;
        DSCONLbits.RELEASE = 0;    // release control and data bits for all I/Os
        WDTCONbits.DS = 0;       // clear the deep-sleep status bit
    }
    
    /*******************************************************************/
    // Initialize Hardware
    /*******************************************************************/
    SYSTEM_Initialize();
    
    LED0 = LED1 = LED2 = 1;
 	
    Read_MAC_Address();
    
    /*******************************************************************/
    // Display Start-up Splash Screen
    /*******************************************************************/

#if DEBUG
    LCD_Erase();
    sprintf((char *)LCDText, (char*)"    Baldr       "  );
    sprintf((char *)&(LCDText[16]), (char*)"  Demo Board    ");
    LCD_Update();
#endif
    
    MiApp_ProtocolInit(false);
    
#if DEBUG
    if (ds_wake)
    {
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"Deep Sleep wake:"  );

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
    }
#endif

    LED0 = 0;
    
    enter_deep_sleep();
    
    LED1 = 0; //TODO: this actually occasionally executes
}

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
//#if DEBUG
//    LCD_Erase();
//    sprintf((char *)LCDText, "%03d,%03d,%03d,%03d",testOutput[0],testOutput[1],testOutput[2],testOutput[3]);
//    sprintf((char *)&(LCDText[16]), (char*)"Memory Test     ");
//    LCD_Update();
//#endif
//    
//    DELAY_ms(10000);
//    
//}

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
    uint8_t i, j, k;
    bool ds_wake = false;
    bool alert = false;
    volatile uint8_t scanresult;
    bool result = true;
    uint8_t switch_val;
    uint8_t pktCMD = 0;

    if (WDTCONbits.DS)   // Woke up from deep sleep
    {
        DSCONLbits.RELEASE = 0;    // release control and data bits for all I/O ports
        WDTCONbits.DS = 0;       // clear the deep-sleep status bit
        ds_wake = true;
    }
    
    SYSTEM_Initialize();
#if DEBUG
    LCD_Initialize();
#endif

    /*******************************************************************/
    // Initialize Hardware
    /*******************************************************************/
    
    
    LED0 = 1;
    LED1 = 0;
    LED2 = 0;
    
    Read_MAC_Address();

    /*******************************************************************/
    // Initialize the MiWi Protocol Stack. The only input parameter indicates
    // if previous network configuration should be restored.
    /*******************************************************************/
    //TODO: I'm not sure exactly what the parameter does
    MiApp_ProtocolInit(false);
    
    /*******************************************************************/
    // Display Start-up Splash Screen
    /*******************************************************************/
#if DEBUG
    if(ds_wake)
    {
        LCD_BacklightON();
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"    Wake        "  );
        
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
        LCD_BacklightOFF();
    }
    else
    {
        LCD_BacklightON();
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"    Baldr       "  );
        sprintf((char *)&(LCDText[16]), (char*)"  Demo Board    ");
        LCD_Update();
        LCD_BacklightOFF();
    }
    
    delay_ms(300);
#endif
    
    //we woke because a sensor was tripped, notify of intrusion
    if(ds_wake && DSWAKEHbits.DSINT0)
    {
        alert = true;  
    }

    //TODO: put the sensor loop here
    
    //TODO: have the user able to set channel?
    
    //TODO: get myChannel from non-volatile memory
    if( MiApp_SetChannel(myChannel) == false )
    {
#if DEBUG
        LCD_Display((char *)"ERROR: Unable toSet Channel..", 0, true);
#endif
        return;
    }
    
    //TODO: I don't think we need to do this if we call protocolInit
    //MiApp_TransceiverPowerState(POWER_STATE_WAKEUP);
    
    /*******************************************************************/
    //  Set the connection mode. The possible connection modes are:
    //      ENABLE_ALL_CONN:    Enable all kinds of connection
    //      ENABLE_PREV_CONN:   Only allow connection already exists in 
    //                          connection table
    //      ENABL_ACTIVE_SCAN_RSP:  Allow response to Active scan
    //      DISABLE_ALL_CONN:   Disable all connections. 
    /*******************************************************************/
    MiApp_ConnectionMode(ENABLE_ALL_CONN);

#if DEBUG
    LCD_Display((char *)"  Scanning for    Networks....", 0, true);
#endif
    
    while(result)
    {
        /*******************************************************************/
        // Perform an active scan
        /*******************************************************************/
        if(myChannel < 8)
            scanresult = MiApp_SearchConnection(10, (0x00000001 << myChannel));
        else if(myChannel < 16)
            scanresult = MiApp_SearchConnection(10, (0x00000100 << (myChannel-8)));
        else if(myChannel < 24)
            scanresult = MiApp_SearchConnection(10, (0x00010000 << (myChannel-16)));
        else
            scanresult = MiApp_SearchConnection(10, (0x01000000 << (myChannel-24)));

        LED0 = 0;
        LED1 = 1;
        LED2 = 0;

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

    #if DEBUG
                // Display the index on LCD
                LCD_Erase();

                // Display if Network is Cordinator or PAN Cordinator
                sprintf((char *)LCDText, (char*)"SW1:<PANID:%02x%02x>",ActiveScanResults[j].PANID.v[1], ActiveScanResults[j].PANID.v[0]);
                //sprintf((char *)&(LCDText[16]), (char*)"SW2: Additional");

                LCD_Update();
    #endif

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
                    MiApp_WriteData(IDENTIFY_MODE);
                    MiApp_WriteData(ActiveScanResults[j].PANID.v[1]);
                    MiApp_WriteData(ActiveScanResults[j].PANID.v[0]);
                    MiApp_BroadcastPacket(false);
                    for(k = 0; k < scanresult; k++)
                    {
                        if((ActiveScanResults[j].PANID.v[1] == ActiveScanResults[k].PANID.v[1]) &
                            (ActiveScanResults[j].PANID.v[0] == ActiveScanResults[k].PANID.v[0]))
                        {
    #if DEBUG
                            LCD_Erase();

                            // Display Network information
                            sprintf((char *)LCDText, (char*)"SW1:<Addr:%02x%02x>",ActiveScanResults[k].Address[1], ActiveScanResults[k].Address[0]);
                            sprintf((char *)&(LCDText[16]), (char*)"SW2: Additional");

                            LCD_Update();
    #endif

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
    #if DEBUG
                    LCD_Display((char *)"Join Failed!!!", 0, true);
    #endif
                    //TODO: this currently just loops and tries again, should we do something more intelligent?
                }
                else
                {
    #if DEBUG
                    LCD_Display((char *)"Joined  Network Successfully..", 0, true);
    #endif
                    MiApp_FlushTx();
                    MiApp_WriteData(EXIT_IDENTIFY_MODE);
                    MiApp_WriteData(myPANID.v[1]);
                    MiApp_WriteData(myPANID.v[0]);
                    MiApp_BroadcastPacket(false);

                    result = false;
                }

                /*******************************************************************/
                // If Connected to a Network Successfully bail out
                /*******************************************************************/
                if(result == false)
                {
                    break;
                }
            }

            result = false;
        }else
        {
            //TODO: there is no network, what do we do?
            result = true;
        }
    }
    
    // Check if MiMAC has any pkt's to processes
    if(MiApp_MessageAvailable())
    {
        pktCMD = rxMessage.Payload[0];
        if(pktCMD == IDENTIFY_MODE)
        {
            if((rxMessage.Payload[1] != myPANID.v[1]) || (rxMessage.Payload[2] != myPANID.v[0]))
                pktCMD = 0;

        }
        MiApp_DiscardMessage();
    }
    
    result = false;

#if 0
    while(result == false)
    {
        // Check if MiMAC has any pkt's to processes
        if(MiApp_MessageAvailable())
        {
            pktCMD = rxMessage.Payload[0];
            if(pktCMD == IDENTIFY_MODE)
            {
                if((rxMessage.Payload[1] != myPANID.v[1]) || (rxMessage.Payload[2] != myPANID.v[0]))
                {
                    pktCMD = 0;
                }

            }
            MiApp_DiscardMessage();
        } else {
            //TODO: there were no messages, what should we do?
            result = false;
        }
        if(pktCMD == SECURITY_DEMO)
        {
            MiApp_FlushTx();
            MiApp_WriteData(SECURITY_DEMO);
            MiApp_BroadcastPacket(false);

            // Run Baldr Security Demo
            SecurityDemo();
            result = true;
        }
        if(pktCMD == IDENTIFY_MODE)
        {
#if DEBUG
            LCD_Erase();
            sprintf((char *)&(LCDText), (char*)"PANID:%02x%02x Ch:%02d",myPANID.v[1],myPANID.v[0],myChannel);
            #if defined(IEEE_802_15_4)
                sprintf((char *)&(LCDText[16]), (char*)"Address: %02x%02x", myShortAddress.v[1], myShortAddress.v[0]);
            #else
                sprintf((char *)&(LCDText[16]), (char*)"Addr:%02x%02x/%02x%02x", myShortAddress.v[1], myShortAddress.v[0], myLongAddress[1],myLongAddress[0]);
            #endif
            LCD_Update();
#endif
            pktCMD = 0;
        }

        if(pktCMD == EXIT_IDENTIFY_MODE)
        {
            result = true;
        }
    }
#endif
    
    DELAY_ms(1000);
    
    LCD_Erase();
    sprintf((char *)&(LCDText), (char*)"PANID:%02x%02x Ch:%02d",myPANID.v[1],myPANID.v[0],myChannel);
    sprintf((char *)&(LCDText[16]), (char*)"Address: %02x%02x", myShortAddress.v[1], myShortAddress.v[0]);
    LCD_Update();
    
    DELAY_ms(5000);
    
    LCD_Display((char *)"Sending ALBATROSS_DEMO Pkt", 0, true);
    bool endDemo = false;
    uint8_t count2 = 0;
    while(!endDemo)
    {
        MiApp_FlushTx();
        if(alert) MiApp_WriteData(DEMO_ALERT);
        else MiApp_WriteData(DEMO_NO_ALERT);
        MiApp_BroadcastPacket(false);
        DELAY_ms(200);
        if(MiApp_MessageAvailable())
        {
            pktCMD = rxMessage.Payload[0];
            if(pktCMD == DEMO_ACK)
            {
                LCD_Display((char *)"Received ACK Packet!", 0, false);
                endDemo = true;
            }else
            {
                LCD_Display((char *)"Received Incorrect Packet!", 0, false);
            }
            DELAY_ms(1000);
            LCD_Erase();
            DELAY_ms(1000);
            MiApp_DiscardMessage();
        }
        
        count2++;
        if(count2 > 10) break;
    }
    
    //put the tranciever to sleep, very important for power saving
    MiApp_TransceiverPowerState(POWER_STATE_SLEEP);

    LED0 = 0;
    LED1 = 0;
    LED2 = 1;
    
    //when the processor wakes it will resume at the start of main()
    enter_deep_sleep();
}

// ISR
void UserInterruptHandler(void)
{
    //check for INT0 interrupt
    if(INTCONbits.INT0IF == 1)
    {
        INTCONbits.INT0IF = 0;
        if(LED2)
        {
            LED2 = 0;
        } else {
            LED2 = 1;
        }
    }
    
    //check for ULP interrupt
    if(INTCON3bits.INT2IF == 1)
    {
        INTCON3bits.INT2IF = 0;
        if(LED1)
        {
            LED1 = 0;
        } else {
            LED1 = 1;
        }
    }
    
    //check if switch 1 was pressed
//    if(INTCON3bits.INT3IF == 1)
//    {
//        INTCON3bits.INT3IF = 0;
//        if(LED0)
//        {
//            LED0 = 0;
//        } else {
//            LED0 = 1;
//        }
//    }
}


