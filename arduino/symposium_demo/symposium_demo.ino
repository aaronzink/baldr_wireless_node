// Written by Nick Gammon
// April 2011

#include "Adafruit_FONA.h"
#include "fydp_camera.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

char callerIDbuffer[32];  

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
// Use this one for FONA 3G
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;


//1 check fona messages
//enable spi interrupts
//respond to command with ready
//lesten to next command
// go into

#define SPI_ARD_READ            0x01
#define SPI_ARD_READ_CONT       0x02
#define SPI_ARD_SEND            0x03
#define SPI_ARD_CHECK_AWAKE     0xF0
#define SPI_ARD_IS_AWAKE        0xF1
#define SPI_ARD_ALLDONE         0xF2
#define SPI_ARD_ALERT           0x04

volatile byte command = 0;
String in_buf = "";
bool handshake = false;
bool allDone = false;
bool redy = false;
bool halt = true;

String mock_sms = "default message";
int counter = 0;
int lastSPDR;
byte picCommand;
byte userCommand;
//bool print=false;

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

void setup (void) {
  Serial.begin (115200);
  Serial.println ("<----online---->");

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  setupFona(); 


  // reading the SMS that is online
  //boolean readSMS(uint8_t i, char *smsbuff, uint16_t max, uint16_t *readsize);
  
  while(!recieveSMS()){
    delay(1000);
  }

  Serial.println("enabling the SPI bus");

  // parse the user message


  redy = true;
  // turn on SPI in slave mode
  SPCR &= 0b00000000;
  SPCR |= 0b11001100;

}

char fonaInBuffer[64]; 

bool recieveSMS() {
  
  char* bufPtr = fonaInBuffer;    //handy buffer pointer
  
  if (fona.available())      //any data available from the FONA?
  {
    int slot = 0;            //this will be the slot number of the SMS
    int charCount = 0;
    //Read the notification into fonaInBuffer
    do  {
      *bufPtr = fona.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaInBuffer)-1)));
    
    //Add a terminal NULL to the notification string
    *bufPtr = 0;
    
    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (1 == sscanf(fonaInBuffer, "+CMTI: \"SM\",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);
      
      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(slot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);
      
      //Send back an automatic response
      Serial.println("Sending reponse...");
      if (!fona.sendSMS(callerIDbuffer, "This number has been added to receive text alerts.")) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Sent!"));
      }
      
      // delete the original msg after it is processed
      //   otherwise, we will fill up all the slots
      //   and then we won't be able to receive SMS anymore
      if (fona.deleteSMS(slot)) {
        Serial.println(F("OK!"));
      } else {
        Serial.println(F("Couldn't delete"));
      }
      return true;
    }
    return false;
  }
}

void setupFona() {
  while (!Serial);

  Serial.begin(115200);
  Serial.println(F("FONA SMS caller ID test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  // make it slow so its easy to read!
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while(1);
  }
  Serial.println(F("FONA is OK"));

  // Print SIM card IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  }
  
  Serial.println("FONA Ready");
}

// SPI interrupt routine
ISR (SPI_STC_vect) {

  byte command = SPDR;
  lastSPDR = command;

  Serial.println(command);

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
    default:
      return false;
      break;
  }
  return true;
}

byte parseUserCommand(char * userSMS) {
  if (true) return 0x01;
  if (false) return 0x02;
  return 0x03;
}

bool sendSMS(char * message, char * sendto ) {
  flushSerial();
  if (!fona.sendSMS(sendto, message)) {
    return true;
  } else {
    return false;
  }
}

bool takePicture() {
  return true; //TODO
}

bool sendPicture() {
  fona.enableGPRS(false);
  fona.enableGPRS(true);

  //return fona.Email_sendEmail();
}

void imageUpdate() { //TODO
  takePicture();

  //fona.Email_sendEmail();
}


void loop (void) {

  if (!halt) {
    Serial.println("escaped halt");

    if (! isValid(picCommand)) {
      picCommand = 0;
      halt = true;

      // enable slave mode
      SPCR &= 0b00000000;
      SPCR |= 0b11001100;
      return;
    }

    // userCommand and Pic command should be both available now;

    // we first do what PIC need
    switch (picCommand) {
      // send status SMS
      case SPI_ARD_READ:
        Serial.println(F("SPI_ARD_READ"));
        sendSMS(callerIDbuffer,"Status: GOOD");
        break;
      // take picture and email
      case SPI_ARD_ALERT:
        Serial.println(F("SPI_ARD_ALERT"));
        sendSMS(callerIDbuffer,"Status: Security alert! Intrusion detected by Node 1 at the group 26 demo.");
        break;
      default:
        Serial.println(F("Wrong SPI Input"));
        //sendSMS("Status: GOOD", "5195882516");
        break;
    }

    // then we do what user asked for
//    if (userCommand != picCommand) {
//      switch (userCommand) {
//        // send status SMS
//        case 1:
//          sendSMS("Status: GOOD", "5195882516");
//          break;
//
//        //take picture and email
//        case 2:
//
//          break;
//
//        default:
//          Serial.println("nothing done");
//          break;
//      }
//    }


    //after everything is done
    //renable the slave mode and send signal to tell pic all done
    allDone = true;

    SPCR &= 0b00000000;
    SPCR |= 0b11001100;

    // waiting to be turned off
    while (1);
  }
}

