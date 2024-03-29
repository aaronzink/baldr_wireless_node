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
#include "parser.h"


// Demo Version
#define MAJOR_REV       1
#define MINOR_REV       3

//use to activate LCD
#define DEBUG_LCD 0
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

#define MiWi_CHANNEL        0x04000000                          //Channel 26 bitmap

//previously packets were assigned up to 5, start at 6
#define ALBATROSS_ACK       6
#define ALBATROSS_ALERT     7
#define ALBATROSS_NO_ALERT  8
#define ALBATROSS_INIT      9
#define ALBATROSS_CONNECT   10

//Test defines
//#define SPI_TEST            1

uint8_t ConnectionEntry = 0;

bool alert = false;

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

//TODO: this should be removed
#if 0 
void test_code()
{
    /*******************************************************************/
    // Testing the parser with:
    // <config-begin>
    // <config-addUser> ckreid asdf1234
    // <config-master> ckreid asdf1234
    // <config-end>
    //
    // Then
    // <config-begin>
    // <config-addUser> luke wrongpass
    // <config-end>
    //
    // Then
    // <config-begin>
    // <config-addUser> luke asdf1234
    // <config-addUser> aaron asdf1234
    // <config-end>
    //
    // Then
    // <config-begin>
    // <config-listUsers>
    // <config-end>
    //
    // Simulates input from 4 separate text messages as displayed above
    /*******************************************************************/
    if( ParseTest )
    {
        char input[150] = "<config-begin>\n<config-addUser> ckreid asdf1234\n<config-master> ckreid asdf1234\n<config-end>\n";
        executeCommands(input);
        char input2[150] = "<config-begin>\n<config-addUser> luke wrongpass\n<config-end>";
        executeCommands(input2);
        char input3[150] = "<config-begin>\n<config-addUser> luke asdf1234\n<config-addUser> aaron asdf1234\n<config-end>";
        executeCommands(input3);
        char input4[150] = "<config-begin>\n<config-listUsers>\n<config-end>";
        executeCommands(input4);
        for( ;; );
    }
    
    if( spiTest )
    {
        SPITest();
    }
    
    if( memTest )
    {
        memoryTest();
    }
    
    if( ardSPITest )
    {
        while(true)
        {
#if DEBUG_LCD
            LCD_Erase();
            sprintf((char *)LCDText, (char*) "PIC to ARD SPI  ");
            sprintf((char *)&(LCDText[16]), (char*) "Read String Test");
            LCD_Update();
#endif
            DELAY_ms(3000);
            
            
            uint8_t myString[150];
            uint8_t count = ARDReadText(&myString);
    
            for(int i = 0; i < (count/32)+1; i++)
            {
#if DEBUG_LCD
                LCD_Erase();
                sprintf((char *)LCDText, myString[i*32]);
                LCD_Update();
#endif
                DELAY_ms(5000);
            }
            
#if DEBUG_LCD
            LCD_Erase();
            sprintf((char *)LCDText, (char*) "PIC to ARD SPI  ");
            sprintf((char *)&(LCDText[16]), (char*) "Send String Test");
            LCD_Update();
            DELAY_ms(3000);
#endif
            
            
            uint8_t * sendString = "This is the reply to the message you forwarded.";
            ARDWriteText(&sendString, 30);
        }
    } 
}
#endif

void set_alert(bool val)
{
    if(val)
    {
        alert = true;
#if DEBUG_LED
        LED0 = LED1 = LED2 = 1;
#endif
    }
    else
    {
        alert = false;
#if DEBUG_LED
        LED1 = LED2 = 0;
#endif
    }
}

void setup_transceiver()
{
    Read_MAC_Address();

    /*******************************************************************/
    // Initialize the MiWi Protocol Stack. The only input parameter indicates
    // if previous network configuration should be restored.
    /*******************************************************************/
    MiApp_ProtocolInit(false);

    if( MiApp_SetChannel(myChannel) == false )
    {
#if DEBUG_LED
        LED0 = LED1 = 0;
        LED2 = 1;
#endif
#if DEBUG_LCD
        LCD_Display((char *)"ERROR: Unable toSet Channel..", 0, true);
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
        scanresult = MiApp_SearchConnection(5, (0x00000001 << myChannel));
    else if(myChannel < 16)
        scanresult = MiApp_SearchConnection(5, (0x00000100 << (myChannel-8)));
    else if(myChannel < 24)
        scanresult = MiApp_SearchConnection(5, (0x00010000 << (myChannel-16)));
    else
    {
        //scantime = 5 seems to be the fastest we can go while still connecting
        scanresult = MiApp_SearchConnection(5, (0x01000000 << (myChannel-24)));
    }

    return scanresult;
}

void setup_network()
{
    uint8_t scanresult;

#if DEBUG_LCD
    LCD_Display((char *)"  Scanning for    Networks....", 0, true);
#endif

    scanresult = scan_for_network();

    /*******************************************************************/
    // Display Scan Results
    /*******************************************************************/
    if(scanresult > 0) //one or more networks exist on this channel
    {
        //TODO: handle this case better
        /* There is an obvious issue, as this should be the only source of
           the network in this sector...
           In future could try a different channel */

#if DEBUG_LCD
        LCD_Display((char *)"Network Exists! Error!", 0, true);
#endif
        
        //make sure the transceiver is sleeping, very important for power saving
        MiApp_TransceiverPowerState(POWER_STATE_SLEEP);
    
#if DEBUG_LCD
        DELAY_ms(100);
        LCD_Erase();
#endif
#if DEBUG_LED
        LED0 = LED1 = LED2 = 0;
#endif
    
        //a network already exists, sleep and try again
        enter_deep_sleep();
    }
    else //there is no existing network
    {
#if DEBUG_LCD
        LCD_Display((char *)"Creating Network", 0, true);
#endif

        MiApp_ProtocolInit(false);
        
        MiApp_StartConnection(START_CONN_DIRECT, 0, 0);
#if DEBUG_LCD
        LCD_Erase();
        sprintf((char *)&(LCDText), (char*)"PANID:%02x%02x Ch:%02d",myPANID.v[1],myPANID.v[0],myChannel);
        sprintf((char *)&(LCDText[16]), (char*)"Address: %02x%02x", myShortAddress.v[1], myShortAddress.v[0]);
        LCD_Update();
#endif
    }
}

void wait_for_connection()
{
#if DEBUG_LED
    MIWI_TICK t1, t2;
    t1 = MiWi_TickGet();
    LED1 = 1;
#endif
    
    /*******************************************************************/
    // Wait for a Node to Join Network
    /*******************************************************************/
    uint8_t pktCMD = 0;
    while(!ConnectionTable[0].status.bits.isValid)
    {
        if(MiApp_MessageAvailable())
        {
            pktCMD = rxMessage.Payload[0];
            MiApp_DiscardMessage();
            if(pktCMD == ALBATROSS_INIT)
            {
                //TODO: respond to first time node
            }
            else if(pktCMD == ALBATROSS_CONNECT)
            {
                //TODO: this is an error, there should not be sensors already on this network
            }
        }
        
#if DEBUG_LED
        t2 = MiWi_TickGet();
        if( MiWi_TickGetDiff(t2, t1) > (2 * TEN_MILI_SECOND) )
        {
            LED1 ^= 1;
            t1 = MiWi_TickGet();
        }
#endif
    }
    
#if DEBUG_LED
    LED1 = 0;
#endif
    
#if DEBUG_LCD
        LCD_Display((char *)"Sensor node     connected!   ", 0, true);
#endif
}

void check_messages()
{
    MIWI_TICK t1, t2;
#if DEBUG_LED
    MIWI_TICK t3, t4;
    t3 = MiWi_TickGet();
    LED1 = 1;
#endif
    t1 = MiWi_TickGet();
    
    bool connected = false;
    while (!connected)
    {
        if(MiApp_MessageAvailable())
        {
            MiApp_DiscardMessage();
        }
        
        //check if there is any connection
        if(ConnectionTable[0].status.bits.isValid)
        {
            connected = true;
            break; //early out in case the timeout goes off
        }
        
        //TODO: this might be able to go faster, may need to also shorten sensor scan time?
        t2 = MiWi_TickGet();
        if( MiWi_TickGetDiff(t2, t1) > (13 * TEN_MILI_SECOND) )
        {
            break;
        }
        
#if DEBUG_LED
        t4 = MiWi_TickGet();
        if( MiWi_TickGetDiff(t4, t3) > (2 * TEN_MILI_SECOND) )
        {
            LED1 ^= 1;
            t3 = MiWi_TickGet();
        }
#endif
    }
    
#if DEBUG_LED
    LED1 = 0;
#endif
    
    if(connected)
    {
#if DEBUG_LED
    LED2 = 1;
#endif
        
        uint8_t pktCMD = 0;
        bool receive = true;
        while(receive)
        {
            //TODO: should we check for multiple messages
            if(MiApp_MessageAvailable())
            {
                pktCMD = rxMessage.Payload[0];
                MiApp_DiscardMessage();
                if(pktCMD == ALBATROSS_INIT)
                {   
                    //TODO: another node is trying to join, respond correctly
                    receive = true;
#if DEBUG_LCD
                    LCD_Display((char *)"Rx:   INIT      Packet          ", 0, false);
#endif
                }
                else if (pktCMD == ALBATROSS_CONNECT)
                {   
                    receive = true;
#if DEBUG_LCD
                    LCD_Display((char *)"Rx: Connect     Packet          ", 0, false);
#endif
                }
                else if(pktCMD == ALBATROSS_NO_ALERT)
                {
                    receive = false;

#if DEBUG_LCD
                    LCD_Display((char *)"Rx:  No Alert   Packet          ", 0, false);
#endif
                }
                else if(pktCMD == ALBATROSS_ALERT)
                {
                    set_alert(true);
                    receive = false;

#if DEBUG_LCD
                    LCD_Display((char *)"Rx:   Alert     Packet    ", 0, false);
#endif
                }
                else if(pktCMD == ALBATROSS_ACK)
                {
                    receive = false;

#if DEBUG_LCD
                    LCD_Display((char *)"Rx:   ACK       Packet      ", 0, false);
#endif
                }
                else //unknown packet
                {
                    receive = false;
#if DEBUG_LCD
                    LCD_Display((char *)"Rx: Unknown     Packet: %02d    ", pktCMD, false);
#endif
                }
            }
        }
        
#if DEBUG_LED
    LED2 = 0;
#endif

#if DEBUG_LED
        t3 = MiWi_TickGet();
        LED2 = 1;
#endif
        t1 = MiWi_TickGet();
        
        uint8_t cmd = 0;
        bool transmit = true;
        bool timeout = true;
        while(1)
        {   
            if(transmit) {
                transmit = false;
                
                MiApp_FlushTx();
                MiApp_WriteData(ALBATROSS_ACK);
                MiApp_BroadcastPacket(false);
            }
            
            if(MiApp_MessageAvailable())
            {
                cmd = rxMessage.Payload[0];
                MiApp_DiscardMessage();

                //check if the packet is still being sent
                if(cmd == pktCMD)
                {
                    //TODO: can we set acknowledge here, or will that just saturate the channel?
                    timeout = false;
                }
                else
                {
                    //TODO: someone else sent a packet, is this an error or should we handle it?
                }
            }
            
            t2 = MiWi_TickGet();
            //TODO: shorten this timeout
            if(MiWi_TickGetDiff(t2, t1) > (61 * ONE_MILI_SECOND))
            {   
                if(timeout)
                {
                    break;
                }
                
                //reset for the next timeout period
                t1 = MiWi_TickGet();
                
                timeout = true;
                transmit = true;
            }
            
#if DEBUG_LED
            t4 = MiWi_TickGet();
            if(MiWi_TickGetDiff(t4, t3) > (2 * TEN_MILI_SECOND))
            {   
                LED2 ^= 1;
                t3 = MiWi_TickGet();
            }
#endif
        }
        
#if DEBUG_LED
    LED2 = 0;
#endif
    }
#if DEBUG_LCD
    else
    {
        LCD_Display((char *)"No Nodes        Detected        ", 0, false);
    }
#endif
    
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
     * LED0: ON = running, OFF = sleeping
     * LED1: blink = waiting to connect, ON = Alert
     * LED2: blink = transmitting, ON = waiting for packet
     * 
     * only LED2: ON = error
    *******************************************************************/
    bool ds_wake = false;
    uint8_t persist0 = 0x00;
    uint8_t persist1 = 0x00;

    if (WDTCONbits.DS)   // Woke up from deep sleep
    {
        WDTCONbits.DS = 0;       // clear the deep-sleep status bit
        persist0 = DSGPR0;
        persist1 = DSGPR1;
        DSCONLbits.RELEASE = 0;    // release control and data bits for all I/O ports
        
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
    
#if DEBUG_LCD
    if(ds_wake)
    {
        LCD_Erase();
        sprintf((char *)LCDText, (char*)"    Wake        "  );
        
        if (DSWAKELbits.DSMCLR)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  MCLR          ");
        }
        else if (DSWAKEHbits.DSINT0)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  INT0          ");
        }
        else if (DSWAKELbits.DSWDT)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  DSWDT         ");
        }
        else if(DSWAKELbits.DSULP)
        {
            sprintf((char *)&(LCDText[16]), (char*)"  ULPWU         ");
        }
        
        LCD_Update();
        DELAY_ms(1000);
    }
    else //first power on
    {
        /*******************************************************************/
        // Display Start-up Splash Screen
        /*******************************************************************/

        LCD_Erase();
        sprintf((char *)LCDText, (char*)"Albatross      ");
        sprintf((char *)&(LCDText[16]), (char*)"Main Board      ");
        LCD_Update();
        
        DELAY_ms(1000);
    }
#endif
    
#ifdef SPI_TEST
    uint8_t spiCount = 0;
    uint8_t spiReceive = 0;
    while(true)
    {
        ARDTest(&spiCount, &spiReceive);

        LCD_Erase();
        sprintf((char *)LCDText, (char*)"SPI Test %02d", spiCount);
        sprintf((char *)&(LCDText[16]), (char*)"Received %02d", spiReceive);
        LCD_Update();
        DELAY_ms(500);
        spiCount++;
    }
#endif
    
    if(ds_wake)
    {
        //TODO: regular use loop
            //sleep twice, then check sensors for any broadcast
                //if message from sensor then notify user
            //sleep for about a minute (26 times) then power the arduino/FONA and check for user input
        
        
        
        uint8_t sleep_counter = persist1;
        bool sleep_toggle = (persist0 == 0xFF);
        
        //sleep twice, then check sensors for any broadcast
        if(sleep_toggle) {
            //store sleep_toggle == false in the persisted register
            DSGPR0 = 0x00;
#if DEBUG_LCD
            DELAY_ms(100);
            LCD_Erase();
#endif
    
#if DEBUG_LED
            LED0 = LED1 = LED2 = 0;
#endif
            enter_deep_sleep();
        } else {
            setup_transceiver();
            setup_network();
            check_messages();
            
            //put the transceiver to sleep, very important for power saving
            MiApp_TransceiverPowerState(POWER_STATE_SLEEP);
            
            if(alert || sleep_counter == 6) //2.1*2*6 = 25.2 second interval
            {
                ARDAlert(alert);
                
                sleep_counter = 0;
            } else {
                ++sleep_counter;
            }
            
            //store sleep_counter in the persisted register
            DSGPR1 = sleep_counter;
            
            set_alert(false);
        }
    }
    else //first power on
    {
        //TODO: setup loop
            //allow nodes to be added
            //send user feedback

        setup_transceiver();
        setup_network();
        wait_for_connection();
        
        //put the transceiver to sleep, very important for power saving
        MiApp_TransceiverPowerState(POWER_STATE_SLEEP);
    }
    
    //make sure the transceiver is sleeping, very important for power saving
    MiApp_TransceiverPowerState(POWER_STATE_SLEEP);
    
    //store sleep_toggle == true in the persisted register
    DSGPR0 = 0xFF;
    
#if DEBUG_LCD
    DELAY_ms(100);
    LCD_Erase();
#endif
    
#if DEBUG_LED
    LED0 = LED1 = LED2 = 0;
#endif
    
    enter_deep_sleep();
}

void UserInterruptHandler(void)
{
    //TODO: if we need to use the INT0 interrupt then comment out line 184 (#define USE_IRQ0_AS_INTERRUPT)
}
