// Used to get time and power info for camera

// This is a basic snapshot sketch using the VC0706 library.
// On start, the Arduino will find the camera and SD card and
// then snap a photo, saving it to the SD card.

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>

// comment out this line if using Arduino V23 or earlier
#include <SoftwareSerial.h>              

// SD card chip select line varies among boards/shields:
// Adafruit SD shields and modules: pin 10
// Arduino Ethernet shield: pin 4
// Sparkfun SD shield: pin 8
#define chipSelect 10

// Pins for camera connection are configurable.
// With the Arduino Uno, etc., most pins can be used, except for
// those already in use for the SD card (10 through 13 plus
// chipSelect, if other than pin 10).
// With the Arduino Mega, the choices are a bit more involved:
// 1) You can still use SoftwareSerial and connect the camera to
//    a variety of pins...BUT the selection is limited.  The TX
//    pin from the camera (RX on the Arduino, and the first
//    argument to SoftwareSerial()) MUST be one of: 62, 63, 64,
//    65, 66, 67, 68, or 69.  If MEGA_SOFT_SPI is set (and using
//    a conventional Arduino SD shield), pins 50, 51, 52 and 53
//    are also available.  The RX pin from the camera (TX on
//    Arduino, second argument to SoftwareSerial()) can be any
//    pin, again excepting those used by the SD card.
// 2) You can use any of the additional three hardware UARTs on
//    the Mega board (labeled as RX1/TX1, RX2/TX2, RX3,TX3),
//    but must specifically use the two pins defined by that
//    UART; they are not configurable.  In this case, pass the
//    desired Serial object (rather than a SoftwareSerial
//    object) to the VC0706 constructor.

// Using SoftwareSerial (Arduino 1.0+)
// On Uno: camera TX connected to pin 2, camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(6, 7);

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

void setup() {
    int32_t time = millis();


  // When using hardware SPI, the SS pin MUST be set to an
  // output (even if not connected or used).  If left as a
  // floating input w/SPI on, this can cause lockuppage.
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if(chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#else
  if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif

  Serial.begin(9600);
  Serial.println("XXXXXX VC0706 Camera snapshot test XXXXXX");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  
  // Try to locate the camera
  cam.begin();

  // CAUTION: this line needs to be called for most other camera functions to work
  cam.getVersion();

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  //uint8_t comp_level = 0xFF;
  cam.setCompression(0xFF);
  //Serial.print("Compression level: ");
  //Serial.println(comp_level);
  
  // You can read the size back from the camera (optional, but maybe useful?)
  //uint8_t imgsize = cam.getImageSize();
  //Serial.print("Image size: ");
  //if (imgsize == VC0706_640x480) Serial.println("640x480");
  //if (imgsize == VC0706_320x240) Serial.println("320x240");
  //if (imgsize == VC0706_160x120) Serial.println("160x120");

  //Serial.println("Snap in 3 secs...");
  delay(1000);

  cam.takePicture();
  //if (! cam.takePicture()) 
  //  Serial.println("Failed to snap!");
  //else 
  //  Serial.println("Picture taken!");
    
  Serial.println(millis() - time);
  time = millis();

  // Create an image with the name IMAGExx.JPG
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
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
  Serial.println(" byte image.");

  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    //if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
    //  Serial.print('.');
    //  wCount = 0;
    //}
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();

  time = millis() - time;
  Serial.println("Done!");
  Serial.print(time); Serial.println(" ms elapsed");
}

void loop() {
}

