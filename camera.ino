
// Capture image from camera every ten seconds and send to basestation.
// Could send image as a series of bytes to server over a UDP connection.


// Include local files
#include "wifi.h"

// Include third party libraries
#include <ArduCAM.h>
#include <SPI.h>
#include <Wire.h>


#define OV2640_CAM

// Camera class captures images from an arducam module.
class Camera {

  // Set GPIO16 as the slave select :
  const int CS = 16;

  // Wrapped ArduCAM object
  ArduCAM arducam;

  public:

  // Constructor initializes arducam
  Camera() 
    : arducam(OV2640, CS){
  }

  void init(){

    // Join the two wire interface (TWI) bus
    Wire.begin();

    // set the CS as an output:
    pinMode(CS, OUTPUT);

    // initialize SPI:
    SPI.begin();
    SPI.setFrequency(4000000); //4MHz

    //Check if the ArduCAM SPI bus is OK
    arducam.write_reg(ARDUCHIP_TEST1, 0x55);
    uint8_t temp = arducam.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55){
      Serial.println("SPI1 interface Error!");
      while(1);
    }

    //Check if the camera module type is OV2640
    uint8_t vid, pid;
    arducam.wrSensorReg8_8(0xff, 0x01);
    arducam.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    arducam.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
      Serial.println("Can't find OV2640 module!");
    else
      Serial.println("OV2640 detected.");

    //Change to JPEG capture mode and initialize the OV2640 module
    arducam.set_format(JPEG);
    arducam.InitCAM();
    arducam.OV2640_set_JPEG_size(OV2640_320x240);

    arducam.clear_fifo_flag();
  }

  void capture(){

    // Buffer to store image bytes
    const size_t buffer_size = 4096;
    uint8_t buffer[buffer_size] = {0xFF};
  
    // Start capture
    arducam.clear_fifo_flag();
    arducam.start_capture();

    // Loop until capture finishes
    int total_time = millis();
    while (!arducam.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
    total_time = millis() - total_time;
    Serial.print("Capture time (in milliseconds):");
    Serial.println(total_time, DEC);

    // Check length of arducam fifo buffer
    uint32_t len  = arducam.read_fifo_length();
    Serial.print("Captured image size (in bytes): ");
    Serial.println(len);
    if (len >= MAX_FIFO_SIZE)   //8M
      Serial.println("Over size.");
    if (len == 0 ) //0 kb 
      Serial.println("Size is 0.");
    
    // Copy captured jpeg image to local buffer
    uint8_t temp = 0, temp_last = 0;  // current and last read bytes
    int i = 0;
    bool start_of_image_found = false;
    arducam.CS_LOW();
    arducam.set_fifo_burst();
    while ( len-- )
    {
      // Get next byte
      temp_last = temp;
      temp =  SPI.transfer(0x00);

      // If find jpeg end of image code 0xFFD9
      if ( (temp == 0xD9) && (temp_last == 0xFF) )
      {
        // save the last 0xD9
        buffer[i++] = temp;       
        
        // Write the remaining bytes in the buffer
        Serial.println("End of image so write out buffer");
        
        // Stop reading from camera buffer and break while loop
        arducam.CS_HIGH();
        break; 
      }

      // If start of image previously found 
      if(start_of_image_found)
      {
        // Copy image byte to local buffer if not full
        if (i < buffer_size)
          buffer[i++] = temp;
        else
        {
          // Write buffer
          Serial.println("Buffer full so write out");
          i = 0;
          buffer[i++] = temp;
        }
      }
      
      // else check for start of image code 0xFFD8
      else if ((temp == 0xD8) & (temp_last == 0xFF))
      {
        start_of_image_found = true;
        buffer[i++] = temp_last;
        buffer[i++] = temp;  
      }
    } 
    
  }


};

// Instantiate camera object
Camera cam;

// Instantiate wifi object
Wifi wifi;

// Time to sleep
const int sleep_time_in_seconds = 10;

// Arduino sketch setup function is called when this sketch starts running.
void setup() {

  // Open serial port and print startup message
  Serial.begin(115200);
  Serial.println(F("PropertyCam starting."));

  // Initialize camera
  cam.init();

  // Connect to wifi
  wifi.connect();
}

// Arduino loop function called repeatedly while sketch is running
void loop() {

  // Capture photo
  cam.capture();

  // Send photo to basestation
  
  // Pause program for sleep time.  
  // TODO: Figure out how to sleep instead to conserve power
  static size_t delay_count = 0;
  Serial.print("Delay ");
  Serial.println(++delay_count);
  Serial.println();
  delay(sleep_time_in_seconds * 1000);

}
