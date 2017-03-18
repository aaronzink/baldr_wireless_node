// This is a basic snapshot sketch using the VC0706 library but 
// it has been turned into a snapshot funciton that sets the Arduino
// back to how it was on exit.
// On start, the Arduino will find the camera and SD card and
// then snap a photo, saving it to the SD card.

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>              

void capture_then_save();