/*
 * File:   arduionSPI.c
 * Author: Clayton Reid
 *
 * Created on February 22, 2017, 1:56 PM
 */

#include "system.h"
#include "system_config.h"


#define SPI_ARD_READ            0x01
#define SPI_ARD_READ_CONT       0x02
#define SPI_ARD_SEND            0x04
#define SPI_ARD_CHECK_AWAKE     0xF0
#define SPI_ARD_IS_AWAKE        0xF1

/*********************************************************************
* Function:         ARDCheckAwake()
*
* PreCondition:     none
*
* Input:            none
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Should return 0xF1 (SPI_IS_AWAKE) if is awake
*                    
*
* Note:			    
**********************************************************************/ 
uint8_t ARDCheckAwake()
{
    DELAY_ms(10);
    SPIPut2(SPI_ARD_CHECK_AWAKE);
    DELAY_ms(10);
    uint8_t check = SPIGet2();
    DELAY_ms(10);
    return check;
}

/*********************************************************************
* Function:         ARDReadByte(uint8_t *dest)
*
* PreCondition:     none
*
* Input:            uint8_t *dest  - Destination buffer.
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Follow ARDRead to get the next byte
*                    
*
* Note:			    
**********************************************************************/   
void ARDReadByte(uint8_t *dest)
{
    DELAY_ms(10);
    SPIPut2(SPI_ARD_READ);
    DELAY_ms(20);
    *dest = SPIGet2();
    DELAY_ms(10);
}

/*********************************************************************
* Function:         ARDReadByteCont(uint8_t *dest)
*
* PreCondition:     none
*
* Input:            uint8_t *dest  - Destination buffer.
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Follows ARDRead to get the next byte.
*                    
*
* Note:			    
**********************************************************************/   
void ARDReadByteCont(uint8_t *dest)
{
    DELAY_ms(10);
    SPIPut2(SPI_ARD_READ_CONT);
    DELAY_ms(20);
    *dest = SPIGet2();
    DELAY_ms(10);
}

/*********************************************************************
* Function:         ARDReadText(uint8_t *dest)
*
* PreCondition:     none
*
* Input:            uint8_t *dest  - Destination buffer.
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Reads an entire text from the ARD SPI bus
*                    
*
* Note:			    
**********************************************************************/ 
uint8_t ARDReadText(uint8_t *dest)
{
    ARD_nCS = 0;
    DELAY_ms(10);
    while(ARDCheckAwake() != SPI_ARD_IS_AWAKE);
    uint8_t receiveByte;
    ARDReadByte(&receiveByte);
    uint8_t count = receiveByte;
    for(int i = 0; i < count; i++)
    {
        ARDReadByteCont(&receiveByte);
        *dest++ = receiveByte;
    }
    DELAY_ms(10);
    ARD_nCS = 1;
    return count;
}

/*********************************************************************
* Function:         ARDWriteByte(uint8_t *src)
*
* PreCondition:     none
*
* Input:            uint8_t *src - Sending buffer.
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Writes a single byte and discards the echo from
*                   ARD.
*                    
*
* Note:			    
**********************************************************************/ 
void ARDWriteByte(uint8_t *src)
{
    DELAY_ms(10);
    SPIPut2(*src);
    DELAY_ms(10);
    SPIGet2();
    DELAY_ms(10);
}

/*********************************************************************
* Function:         ARDWriteText(uint8_t *src, uint8_t count)
*
* PreCondition:     none
*
* Input:            uint8_t *src - Sending buffer.
*                   uint8_t count - Number of bytes to send. 
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Writes an entire string of data to the ARD on
*                   the SPI bus.
*
* Note:			    
**********************************************************************/ 
void ARDWriteText(uint8_t *src, uint8_t count)
{
    ARD_nCS = 0;
    DELAY_ms(10);
    while(ARDCheckAwake() != SPI_ARD_IS_AWAKE);
    uint8_t myReturn = 0;
    for(int i = 0; i < 1000; i++)
    {
        DELAY_ms(10);
        if(count == 1) SPIPut2(SPI_ARD_SEND);
        else SPIPut2(0x00);
        DELAY_ms(10);
        myReturn = SPIGet2();
        if(myReturn == 0xF2) break;
    }
}



//void ARDTest(uint8_t * count, uint8_t *dest)
//{
//    ARD_nCS = 0;
//    DELAY_ms(10);
//    SPIPut2(*count);
//    DELAY_ms(20);
//    *dest = SPIGet2();
//    DELAY_ms(10);
//    ARD_nCS = 1;
//}
