// Written by Nick Gammon
// April 2011


#include <SPI.h>

void setup (void)
{
  Serial.begin (115200);
  Serial.println ();
  
  digitalWrite(SS, HIGH);  // ensure SS stays high for now

  // Put SCK, MOSI, SS pins into output mode
  // also put SCK, MOSI into LOW state, and SS into HIGH state.
  // Then put SPI hardware into Master mode and turn SPI on
  SPI.begin ();

  // Slow down the master a bit
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  
}  // end of setup

void transferAndWait (const byte what)
{
  byte a = SPI.transfer (what);
  delayMicroseconds (100);
} // end of transferAndWait

void loop (void)
{

  char c;
  // enable Slave Select
  digitalWrite(SS, LOW);    
 
  // send test string
  for (const char * p = "~Would You Kindly?" ; c = *p; p++)
     transferAndWait (c);

  // disable Slave Select
  digitalWrite(SS, HIGH);
  
  delay (1000);  // 1 second delay 
}  // end of loop
