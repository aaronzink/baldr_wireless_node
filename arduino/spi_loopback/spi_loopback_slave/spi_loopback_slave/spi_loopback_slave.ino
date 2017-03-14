// Written by Nick Gammon
// April 2011

volatile byte command = 0;
String in_buf = "";
bool fin = false;
bool redy = false;
String mock_sms = "default message";
int counter = 0;

void setup (void) {
  Serial.begin (115200);
  Serial.println ("<----online---->");

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // getting the fona online


  // reading the SMS that is online
  

  // turn on SPI in slave mode
  SPCR &= 0b00000000;
  SPCR |= 0b11001100;

  
//  SPCR |= _BV(SPE);
//  SPCR |= _BV(SPIE);
}

// SPI interrupt routine
ISR (SPI_STC_vect) {

  char command = SPDR;
  
//  if(counter == mock_sms.length()){
//    counter == 0; 
//    redy = false;
//    fin = true;
//  }

  if( command =='r' && !redy) {
    redy = true; 
  }

  if (redy){
    
    
    switch(command){

      
    }
    SPDR &= 0x00;
    SPDR = mock_sms[counter++];
    //Serial.println(SPDR);
  }
  
}

void loop (void) {
  //waiting the system is ready  
  while(!redy){};

}
