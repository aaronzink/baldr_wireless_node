// Written by Nick Gammon
// April 2011

// what to do with incoming data
volatile byte command = 0;
String in_buf = "";

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
  char c = SPDR;

    switch (command)
    {
    // no command? then this is the command
    case 0:
      command = c;
      Serial.print(in_buf);
      in_buf = "";
      break;
  
    // add to incoming byte, return result
    default:
      //Serial.print(c);
      in_buf+=c;
      break;
  
    // subtract from incoming byte, return result
    } // end of switch

}  // end of interrupt service routine (ISR) SPI_STC_vect

void loop (void)
{
  // if SPI not active, clear current command
  if (digitalRead (SS) == HIGH) {
    command = 0;
    
  } 
}  // end of loop
