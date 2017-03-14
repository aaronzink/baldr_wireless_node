/*
 * File:   arduinoSPI.h
 * Author: Clayton Reid
 *
 * Created on February 22, 2017, 1:56 PM
 */

#ifndef _ARD_SPI_H
    #define _ARD_SPI_H


uint8_t ARDReadText(uint8_t *Dest);
void ARDWriteText(uint8_t *Src, uint8_t Count);
//void ARDTest(uint8_t *count, uint8_t *Dest);

#endif
