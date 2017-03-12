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

#include "security_demo.h"
#include "system.h"
#include "system_config.h"
#include "miwi/miwi_api.h"

#define SENSE_SECOND_INTERVAL       1
#define DISPLAY_CYCLE_INTERVAL      1
#define EXIT_DEMO_TIMEOUT           6
#define NUM_TEMP_SAMPLES            1
#define SENSOR_COUNT                1

#define EXIT_PKT                    1
#define SENSE_PKT                   3
#define ACK_PKT                     4

unsigned short tempAverage = 0;
uint8_t CurrentNodeIndex = 0;

extern bool NetFreezerEnable;
extern uint8_t role;

struct SensorPacket
{
    uint8_t NodeAddress[2];
    uint8_t SensorFlags[SENSOR_COUNT];
}; 
struct SensorPacket NodeSensors[10];



/*********************************************************************
* Function:         void SecurityDemo(void)
*
* PreCondition:     none
*
* Input:            none
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Following routine:
*                   - reads the motion and switch sensors
*                   - Broadcasts these values to the network
*                   - displays the switch states of other Nodes on the network
*                   - activate LED's and the buzzer when motion is detected on the network
*
* Note:			    
**********************************************************************/
void SecurityDemo(bool isSensor)
{
    bool Run_Demo = true;
    uint8_t SensorFlags[4];
    MIWI_TICK tick1, tick2, tick3;
    uint8_t switch_val;
      
	/*******************************************************************/
    // Display Temp Demo Splash Screen
    /*******************************************************************/	
    LCD_Display((char *)"   Baldr           Sense Demo ", 0, true);

    /*******************************************************************/
    // Initialize Temp Data Packet
    // NodeTemp[0] = Self
    /*******************************************************************/
    NodeSensors[0].NodeAddress[0] = myShortAddress.v[0];
    NodeSensors[0].NodeAddress[1] = myShortAddress.v[1];
    
    /*******************************************************************/
    // Read Start tickcount
    /*******************************************************************/	
    tick1 = MiWi_TickGet();
    tick3 = tick1;
    
    while(Run_Demo)
    {
        /*******************************************************************/
        // Read current tickcount
        /*******************************************************************/
		tick2 = MiWi_TickGet();
		
        /*******************************************************************/
        // Check if User wants to Exit Demo by pressing button 2
        /*******************************************************************/
        switch_val = BUTTON_Pressed();
	
        if(0)//switch_val == SW2) //BALDR edit, we don't want to exit the demo
        {
            /*******************************************************************/
        	// Send Exit Demo Request Packet and exit Temp Demo
        	/*******************************************************************/ 
            MiApp_FlushTx();    
            MiApp_WriteData(EXIT_PKT);
            MiApp_BroadcastPacket(false);
            LCD_Display((char *)"   Exiting....     Baldr Demo ", 0, true);           
            
            /*******************************************************************/
            // Wait for ACK Packet or Timeout
            /*******************************************************************/
            tick1 = MiWi_TickGet();
            while(Run_Demo)
            {
                if(MiApp_MessageAvailable())
                {
                    if(rxMessage.Payload[0] == ACK_PKT)          
                        Run_Demo = false;
                        
                    MiApp_DiscardMessage();
                }
                if ((MiWi_TickGetDiff(tick2,tick1) > (ONE_SECOND * EXIT_DEMO_TIMEOUT)))
                    Run_Demo = false;
                    
                tick2 = MiWi_TickGet();
            }  
        } 
            
        /*******************************************************************/
        // Rotate through Displaying All Node Temp's
        /*******************************************************************/
        if ((MiWi_TickGetDiff(tick2,tick3) > (ONE_SECOND * DISPLAY_CYCLE_INTERVAL)))
        {
    		/*if((ConnectionTable[CurrentNodeIndex].status.bits.isValid))
    		{
    		    CurrentNodeIndex++;
    		}    
    		else
    		{
    		    CurrentNodeIndex = 0;
    		}   */ 
            CurrentNodeIndex = 1;
    		
    		PrintAlertLCD();
    		
    		tick3 = MiWi_TickGet();
        }     
		
        /*******************************************************************/
        // Read the Sensors every SENSE_SECOND_INTERVAL
        /*******************************************************************/
        if ((MiWi_TickGetDiff(tick2,tick1) > (ONE_MILI_SECOND * 200 * SENSE_SECOND_INTERVAL)))
        {
			
            /*******************************************************************/
            // Power-up Temperature Sensor
            /*******************************************************************/
            //LATAbits.LATA0 = 1;
            //DELAY_ms(2);         // Delay 2 ms after powering up the temperature sensor
            
            /*******************************************************************/
            // Toggle LED1 Every Temp Read Cycle
            /*******************************************************************/
            //LED0 ^= 1;
            
            /*******************************************************************/
            // Take specified number of Temp Readings
            /*******************************************************************/
            ReadSecuritySensors(SensorFlags, SENSOR_COUNT);

            /*******************************************************************/
            // Turn Temp Sensor Off
            /*******************************************************************/
            //LATAbits.LATA0 = 0;

            /*******************************************************************/
            // Calculate Average Temp
            /*******************************************************************/			
            //tempAverage = 0;
            //tempAverage = (uint8_t)temp;
			
            /*******************************************************************/
            // Reset TX Buffer Pointer
            /*******************************************************************/			
            MiApp_FlushTx();
            
            /*******************************************************************/
            // Write this Nodes temperature value and Address to the TX Buffer
            /*******************************************************************/
            MiApp_WriteData(SENSE_PKT);
            MiApp_WriteData(SensorFlags[0]);
            MiApp_WriteData(SensorFlags[1]);
            MiApp_WriteData(myShortAddress.v[0]);
            MiApp_WriteData(myShortAddress.v[1]);
            
            // Update NodeTemp Structure
            NodeSensors[0].SensorFlags[0] = SensorFlags[0];
            //NodeSensors[0].SensorFlags[1] = SensorFlags[1];
					
           /*******************************************************************/
           // Broadcast Node Tempature across Network.
           /*******************************************************************/
           // Function MiApp_BroadcastPacket is used to broadcast a message
           // The only parameter is the boolean to indicate if we need to
           // secure the frame
           /*******************************************************************/
           MiApp_BroadcastPacket(false);

            /*******************************************************************/
            // Read New Start tickcount
            /*******************************************************************/
            tick1 = MiWi_TickGet();
	}

        /*******************************************************************/
        // Check for Incomming Recieve Packet.
        /*******************************************************************/
        // Function MiApp_MessageAvailable returns a boolean to indicate if 
        // a packet has been received by the transceiver. If a packet has 
        // been received, all information will be stored in the rxFrame, 
        // structure of RECEIVED_MESSAGE.
        /*******************************************************************/
        if(MiApp_MessageAvailable())
        {
            uint8_t i;
            
            /*******************************************************************/
            // Check if Exit Demo Packet
            /*******************************************************************/
            if(rxMessage.Payload[0] == EXIT_PKT)
            {
            MiApp_DiscardMessage();
            MiApp_FlushTx();
            MiApp_WriteData(ACK_PKT);
            MiApp_UnicastConnection(0, false);
            Run_Demo = false;
            LCD_Display((char *)"   Exiting....     Baldr Demo ", 0, true);
            }
            
            /*******************************************************************/
            // Check if Message from known Connection
            /*******************************************************************/
            for(i = 0; i < CONNECTION_SIZE; i++)
            {
                    if((ConnectionTable[i].status.bits.isValid) && (ConnectionTable[i].AltAddress.v[0] == rxMessage.Payload[2]) && (ConnectionTable[i].AltAddress.v[1] == rxMessage.Payload[3]))
                    {
                            if(rxMessage.Payload[0] == SENSE_PKT)
                            {
                                    // Update the Remote Nodes Temp value
                                NodeSensors[i+1].SensorFlags[0] = rxMessage.Payload[1];
                                //NodeSensors[i+1].SensorFlags[1] = rxMessage.Payload[2];
                                NodeSensors[i+1].NodeAddress[0] = rxMessage.Payload[2];
                                NodeSensors[i+1].NodeAddress[1] = rxMessage.Payload[3];
                            }	
                    }
            }

            MiApp_DiscardMessage();
        }
    }   
}


/*********************************************************************
* Function:         uint8_t ReadTempSensor(uint16_t VBGResult)
*
* PreCondition:     Proper reference voltage value has been determined.
*
* Input:            uint16_t VBGResult - Reference voltage for temp calculation.
*
* Output:           uint8_t temp
*
* Side Effects:	    none
*
* Overview:         Following routine reads the on board Tempature Sensor and
*                   calculates the temp value. 
*
* Note:			    
**********************************************************************/
void ReadSecuritySensors(uint8_t *readings, uint8_t count)
{
    //uint8_t switch_val = BUTTON_Pressed();
    readings[0] = AUX1_PORT + AUX2_PORT;//(switch_val == SW1)? 1 : 0;
   // readings[1] = AUX2_PORT;

}

/*********************************************************************
* Function:         void PrintTempLCD(void)
*
* PreCondition:     none
*
* Input:            none
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Converts the Node Temp reading to Feirenhiet and
*                   display's it on the LCD.
*
* Note:			    
**********************************************************************/
void PrintAlertLCD(void)
{
    uint8_t sense1 = NodeSensors[CurrentNodeIndex].SensorFlags[0];
    uint8_t sense2 = NodeSensors[CurrentNodeIndex].SensorFlags[1];

    LCD_Erase();
    
    if((sense1 != 0) || (sense2 != 0))
	{
		sprintf((char *)LCDText, (char*)"Alert Detected    ");
        sprintf((char *)&(LCDText[16]), (char*)"Texting User    ");
        BUZZER_PORT = 1;
        LED2 = 1; //Red LED labeled LED3
	}
    else
	{
    	sprintf((char *)LCDText, (char*)"  Fine  ");
        BUZZER_PORT = 0;
        LED2 = 0;
 	}
    
    LCD_Update();
}
