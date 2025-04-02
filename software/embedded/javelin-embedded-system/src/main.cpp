#include <Arduino.h>

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20649.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <AHRS.h>
#include <SD.h>
#include <FS.h>
#include <EEPROM.h>

uint16_t frameCounter = 0;
QueueHandle_t queue = NULL;

AHRS ahrs;
icm20x_raw_axes_t raw_axesD[ICM20X_FIFO_SIZE/6];
File myFile;

volatile bool interruptOccurred = false;

void IRAM_ATTR handleInterrupt() {
  interruptOccurred = true;
}

String getFileName()
{
  //get timesttamp function in a certain format
  //future implementation with RTC
  long r = random(100000,999999);
  String fileName = "/"+ String(r) + ".csv";
  return fileName;
}

state_t ahrs_state = AHRS_WAITING;

unsigned long startTime = 0;
#define EEPROM_SIZE 64


int threshold = 40;
bool touchActive = false;
bool lastTouchActive = false;
bool testingLower = true;

void gotTouchEvent() {
  if (lastTouchActive != testingLower) {
    touchActive = !touchActive;
    testingLower = !testingLower;
    // Touch ISR will be inverted: Lower <--> Higher than the Threshold after ISR event is noticed
    touchInterruptSetThresholdDirection(testingLower);
  }
}

void setup(void) {
  Serial.begin(115200);//start serial for debugging

  touchAttachInterrupt(T9, gotTouchEvent, threshold);

  if (!EEPROM.begin(EEPROM_SIZE)) {//initialize EEPROM
    Serial.println("failed to initialize EEPROM");
    delay(1000000);
  }

  if (!SD.begin()) {// initialize SD card
    Serial.println("SD card initialization failed!");
    while (true) {
      delay(10);
    }
  }
  queue = xQueueCreate(500, sizeof(ahrs_axes_t));//create queue for 500 elements of type ahrs_axes_t
  ahrs.beginAHRSi2c(); // Initialize ICM20649 and LIS3MDL
  ahrs.configAHRS();// Configure ICM20649 and LIS3MDL
  pinMode(4,INPUT);
  attachInterrupt(digitalPinToInterrupt(4), handleInterrupt, RISING); //attach interrupt to pin 4

  //ahrs.lowPowerMode(true);
  //Serial.println("Low power mode active....");
  //delay(2000);
  //ahrs.lowPowerMode(false);//wake ahrs
  //Serial.println("Low power mode deactive....");
  //delay(2000);
}

void loop() {

  switch (ahrs_state)
  {
  case AHRS_WAITING:
    if (lastTouchActive != touchActive) {
      lastTouchActive = touchActive;
      if (touchActive) {
        Serial.println("  ---- Touch was Pressed");
        ahrs_state = AHRS_PREP;
      }
    }
    break;
  case AHRS_PREP:
  {
  String fname = getFileName();
    myFile = SD.open(fname, FILE_WRITE);//open file for writing
    if (!myFile) {
      Serial.println("Error opening datalog.txt");
      ahrs_state = AHRS_WAITING;
    } else
    {
      ahrs_state = AHRS_SAMPLING;
      ahrs.icm20649.resetFIFO();
      xQueueReset(queue);
      frameCounter = 0;
      //ahrs.lowPowerMode(false);//wake ahrs
    }
    Serial.println("Begin sampling in file" + fname);
    startTime = millis();
    break;
  }
  case AHRS_SAMPLING:
  {
    if (interruptOccurred) {
      interruptOccurred = false;
      //unsigned long t0 = millis();
      uint16_t frameCount = ahrs.icm20649.readFIFOBuffer(raw_axesD);
      //Serial.println("Time to transfer fifo: "+ String(millis()-t0));
      for (size_t i = 0; i < frameCount; i++) {
        ahrs_axes_t scaled_axes = ahrs.scaleAxes(raw_axesD[i]);
        if (xQueueSend(queue, &scaled_axes, portMAX_DELAY) != pdPASS) {
          Serial.println("Queue full, frame dropped");
        }
        frameCounter++;
      }
     
    }
    ahrs_axes_t scaled_axes;
    if (xQueueReceive(queue, &scaled_axes, 0) == pdPASS) {
        String data = String(scaled_axes.accX, 4) + "," +
                      String(scaled_axes.accY, 4) + "," +
                      String(scaled_axes.accZ, 4) + "," +
                      String(scaled_axes.gyroX, 4) + "," +
                      String(scaled_axes.gyroY, 4) + "," +
                      String(scaled_axes.gyroZ, 4) + "," +
                      String(scaled_axes.magX, 4) + "," +
                      String(scaled_axes.magY, 4) + "," +
                      String(scaled_axes.magZ, 4) + "\n";
        myFile.print(data);
    }
    if(millis() - startTime > 10000){
      while (xQueueReceive(queue, &scaled_axes, 0) == pdPASS) {
        String data = String(scaled_axes.accX, 4) + "," +
                      String(scaled_axes.accY, 4) + "," +
                      String(scaled_axes.accZ, 4) + "," +
                      String(scaled_axes.gyroX, 4) + "," +
                      String(scaled_axes.gyroY, 4) + "," +
                      String(scaled_axes.gyroZ, 4) + "," +
                      String(scaled_axes.magX, 4) + "," +
                      String(scaled_axes.magY, 4) + "," +
                      String(scaled_axes.magZ, 4) + "\n";
        myFile.print(data);
    }
      ahrs_state = AHRS_END;
    }
    break;
  }
  case AHRS_END:
    myFile.close();
    Serial.println("Datalogging ended");
    //ahrs.lowPowerMode(true);
    Serial.print("Frames read: ");
    Serial.println(frameCounter);
    ahrs_state = AHRS_WAITING;
    break;
  
  default:
    ahrs_state = AHRS_END;
    break;
  }

}
