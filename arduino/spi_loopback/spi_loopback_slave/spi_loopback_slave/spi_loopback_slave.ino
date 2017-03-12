// Written by Nick Gammon
// April 2011

#include <SPI.h>

volatile byte command = 0;
String in_buf = "";
bool fin = false;

void setup (void)
{
  Serial.begin (115200);
  Serial.println ("<----online---->");

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // turn on interrupts
  SPCR |= _BV(SPIE);

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  noInterrupts();
  char c = SPDR;
  //    Serial.print(c);
  //    in_buf+=c;
  if (! fin) {
    switch (c)
    {
      case '{':
        in_buf = "";
        break;

      case '}':
        fin = true;
        break;

      default:
        //Serial.print(c);
        in_buf += c;
        break;
    }

    if (!fin) interrupts();
    else {
      Serial.print("Received Command: ");
      Serial.println(in_buf);
    }
  }
}  // end of interrupt service routine (ISR) SPI_STC_vect

char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1 << SPIF)))   // Wait for the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}

void loop (void)
{
  // if SPI not active, clear current command
  while (digitalRead (SS) == LOW) {
  }
  while (digitalRead (SS) == HIGH) {
  }
  while (digitalRead (SS) == LOW) {
  }

  Serial.println("Done Receiving");
  Serial.println("Sending debug info.");


  SPI.end();
  delay(100);
  SPI.begin();
 
  Serial.print(SPI.transfer((byte) '4'));
  Serial.print(SPI.transfer((byte)'t'));
  Serial.print(SPI.transfer((byte)'e'));
  Serial.print(SPI.transfer((byte)'s'));
  Serial.print(SPI.transfer((byte)'t'));
  Serial.print('\n');

  while (1);

}  // end of loop
