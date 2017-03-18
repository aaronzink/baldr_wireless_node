// This is a basic snapshot sketch using the VC0706 library but 
// it has been turned into a snapshot funciton that sets the Arduino
// back to how it was on exit.
// On start, the Arduino will find the camera and SD card and
// then snap a photo, saving it to the SD card.

#include "fydp_camera.h"             

void capture_then_save() {
 
  SoftwareSerial cameraconnection = SoftwareSerial(6,7);
  
  Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

  //remember existing settings
  //DDR is the digital pin input/output mode 
  // included in this is the CS pin, MOSI, MISO. etc.
  byte stateDirB = DDRB;
  byte stateDirC = DDRC;
  byte stateDirD = DDRD;
  byte statePortB = PORTB;
  byte statePortC = PORTC;
  byte statePortD = PORTD;
  byte stateSPCR = SPCR;

  // When using hardware SPI, the SS pin MUST be set to an
  // output (even if not connected or used).  If left as a
  // floating input w/SPI on, this can cause lockuppage.
  pinMode(10, OUTPUT);

  Serial.begin(9600);
  Serial.println("VC0706 Camera snapshot test");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  cam.setCompression(255);

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

  Serial.println("Snap in 3 secs...");
  delay(3000);

  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Create an image with the name IMAGExx.JPG
  char filename[14];
  strcpy(filename, "IMAGE000.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/100;
    filename[6] = '0' + (i%100)/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  int32_t time = millis();
  //pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); Serial.println(" ms elapsed");

  //reset system state for registers that were used
  DDRB = stateDirB;
  DDRC = stateDirC;
  DDRD = stateDirD;
  PORTB = statePortB;
  PORTC = statePortC;
  PORTD = statePortD;
  SPCR = stateSPCR;
}
