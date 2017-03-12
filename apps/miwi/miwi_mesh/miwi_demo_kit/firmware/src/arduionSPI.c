/*
 * File:   arduionSPI.c
 * Author: Clayton Reid
 *
 * Created on February 22, 2017, 1:56 PM
 */

#include "system.h"
#include "system_config.h"


#define SPI_READ            0x01
#define SPI_SECTOR_ERASE    0x20
#define SPI_BLOCK_ERASE     0x52
#define SPI_CHIP_ERASE      0x60
#define SPI_BYTE_PROGRAM    0x02
#define SPI_AUTO_ADDRESS_INC 0xAF
#define SPI_READ_STATUS_REG 0x05
#define SPI_WRITE_ENABLE    0x06
#define SPI_READ_ID         0x90
#define SPI_WRITE_DISABLE   0x04

#define READ_MANUFACTURER_ID 0xBF
#define READ_DEVICE_ID      0x49

/*********************************************************************
* Function:         SSTRead(uint8_t *dest, uint8_t *addr, uint8_t count)
*
* PreCondition:     none
*
* Input:            uint8_t *dest  - Destination buffer.
*                   uint8_t *addr   - Address to start reading from.
*                   uint8_t count  - Number of bytes to read.
*
* Output:           none
*
* Side Effects:	    none
*
* Overview:         Following routine reads bytes from the EEProm and puts
*                   them in a buffer.
*                    
*
* Note:			    
**********************************************************************/   
void ARDReadByte(uint8_t *dest)
{
    DELAY_ms(100);
    ARD_nCS = 0;
    DELAY_ms(100);
    SPIPut2(SPI_READ);
    *dest = SPIGet2();
    DELAY_ms(100);
    ARD_nCS = 1;
    DELAY_ms(100);
}

void ARDWrite(uint8_t *src)
{
    DELAY_ms(100);
    ARD_nCS = 0;
    DELAY_ms(100);
    SPIPut2(*src);
    DELAY_ms(100);
    ARD_nCS = 1;
    DELAY_ms(100);
}

void ARDTest(uint8_t *dest)
{
    DELAY_ms(20);
    ARD_nCS = 0;
    DELAY_ms(20);
    SPIPut2('{');
    DELAY_ms(20);
    SPIGet2();
    DELAY_ms(20);
    SPIPut2('a');
    DELAY_ms(20);
    SPIGet2();
    DELAY_ms(20);
    SPIPut2('b');
    DELAY_ms(20);
    SPIGet2();
    DELAY_ms(20);
    SPIPut2('c');
    DELAY_ms(20);
    SPIGet2();
    DELAY_ms(20);
    SPIPut2('}');
    DELAY_ms(20);
    SPIGet2();
    DELAY_ms(20);
    uint8_t count = SPIGet2();
    for(int i = 0; i < count ; i++)
    {
        DELAY_ms(20);
        *dest = SPIGet2();
        if(i != count - 1) *dest++;
    }
    DELAY_ms(20);
    ARD_nCS = 1;
    DELAY_ms(20);
}
