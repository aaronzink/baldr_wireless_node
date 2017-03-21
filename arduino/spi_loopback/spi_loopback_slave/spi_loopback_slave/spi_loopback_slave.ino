// Written by Nick Gammon
// April 2011

volatile byte command = 0;
String in_buf = "";
bool fin = false;
bool redy = false;
int counter = 0;

#define SPI_ARD_READ            0x01
#define SPI_ARD_READ_CONT       0x02
#define SPI_ARD_SEND            0x03
#define SPI_ARD_CHECK_AWAKE     0xF0
#define SPI_ARD_IS_AWAKE        0xF1
#define SPI_ARD_ALLDONE         0xF2
#define SPI_ARD_ALERT           0x04

bool allDone = false;
bool handshake = false;
bool halt = true;

byte picCommand = 0x00;


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
  pinMode(1, OUTPUT);

  redy = true;
  Serial.println ("system online");

  //  SPCR |= _BV(SPE);
  //  SPCR |= _BV(SPIE);
}

// SPI interrupt routine
ISR (SPI_STC_vect) {

  byte command = SPDR;

  //Serial.println(command);

  if (allDone) {
    SPDR = SPI_ARD_ALLDONE;
    return;
  }

  if (handshake) {
    // disable the interrupt
    SPCR &= 0b01111111;

    //take the command, no knowing vaild or not at this point.
    picCommand = command;
    halt = false;
    Serial.println("Command received");
    return;
  }

  if ( command == SPI_ARD_CHECK_AWAKE && redy ) {
    SPDR = SPI_ARD_IS_AWAKE;
    handshake = true;
    Serial.println("Systerm awaken");
    return;
  }


}

bool isValid( byte commandByte) {
  switch (picCommand) {
    case SPI_ARD_READ:
      Serial.println("SPI_ARD_READ");
      break;
    case SPI_ARD_READ_CONT:
      Serial.println("SPI_ARD_READ_CONT");
      break;
    case SPI_ARD_SEND:
      Serial.println("SPI_ARD_SEND");
      break;
    //    case SPI_ARD_CHECK_AWAKE:
    //      Serial.println("SPI_ARD_CHECK_AWAKE");
    //      break;
    //    case SPI_ARD_IS_AWAKE:
    //      Serial.println("SPI_ARD_IS_AWAKE");
    //      break;
    case SPI_ARD_ALLDONE:
      Serial.println("SPI_ARD_ALLDONE");
      break;
    case SPI_ARD_ALERT:
      Serial.println("SPI_ARD_ALERT");
      break;
    default:
      return false;
      break;
  }
  return true;
}

void loop (void) {
  //waiting the system is ready
  if (!halt) {
    Serial.println("Escaped halt");
    Serial.println(picCommand);

    if (! isValid(picCommand)) {
      picCommand = 0;
      halt = true;

      // enable slave mode
      SPCR &= 0b00000000;
      SPCR |= 0b11001100;
      return;
    }
    Serial.println("Command: ");
    Serial.println(picCommand);

    delay(10000);

    digitalWrite(1,HIGH);
    while(1);
  }

}
