/*!
    * @file AHRS.cpp
    * @brief AHRS class definition
*/

#include "AHRS.h"

#define DEBUG_AHRS 0

/**
 * @brief AHRS constructor
 * 
 * This constructor initializes the AHRS object and sets up the I2C communication.
 */
AHRS::AHRS() {}

/**
 * @brief AHRS destructor
 * 
 * This destructor destroys the AHRS object and releases any allocated resources.
 */
AHRS::~AHRS() {}

/**
 * @brief Initialize the I2C communication for AHRS
 * 
 * This function initializes the I2C communication for the ICM20649 and LIS3MDL sensors.
 * 
 * @return uint8_t Status of the initialization (0x00 for success, 0x01 for ICM20649 failure, 0x02 for LIS3MDL failure)
 */
uint8_t AHRS::beginAHRSi2c() {
  Wire.begin(); // Initialize I2C communication
  Wire.setClock(400000); 
  Wire.setBufferSize(250); //este valor debe coincidir con el tamaño del buffer del contructor de la clase Adafruit_I2CDevice. 
                            //Fallo de la libreria por usar un valor hardcodeado. Para dejarlo bien habria que modificar varios metodos de la libreria Adafruit_I2CDevice.cpp y Adafruit_I2CDevice
  uint8_t accelConexionTry = 0;
  uint8_t magConexionTry = 0;

  while (!icm20649.begin_I2C() && accelConexionTry < 3) { // Initialize ICM20649 I2C bus
    #if(DEBUG_AHRS)
      Serial.println("ICM20649 I2C bus initialization failed!");
    #endif
    delay(1000); // Wait for 1 second before retrying
    accelConexionTry++;
  }
  if (!icm20649.begin_I2C())
  {
   return 0x01; // Error code for ICM20649 initialization failure
  }
 
  icm20649.setI2CBypass(true); // Enable I2C bypass

  if (!lis3mdl.begin_I2C() && magConexionTry < 3) { // Initialize LIS3MDL I2C bus
    #if(DEBUG_AHRS)
      Serial.println("LIS3MDL I2C bus initialization failed!");
    #endif
    delay(1000); // Wait for 1 second before retrying
    magConexionTry++;
  }
  if (!lis3mdl.begin_I2C())
  {
    return 0x02; // Error code for LIS3MDL initialization failure
  }

  return 0x00; // Success

}


volatile bool AHRS::inputDataAvailable= false; // Flag for interrupt handling
/**
 * @brief Interrupt handler for data input
 * 
 * This function is called when the interrupt occurs, setting the inputDataAvailable flag to true.
 */
void IRAM_ATTR AHRS::inputDataInterrupt(){inputDataAvailable = true;} // Set the flag to indicate an interrupt has occurred


/**
 * @brief Configure the AHRS sensors
 * 
 * This function configures the ICM20649 and LIS3MDL sensors for data acquisition.
 * 
 * @return bool Status of the configuration (true for success, false for failure)
 */
bool AHRS::configAHRS() {

  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
  //loadMagCalibFromEEPROM(); // Load magnetometer calibration offsets from EEPROM
  
  icm20649.setI2CBypass(true); // Enable I2C bypass
  
  //lis3mdl.writeOffsetxyz();// Write offset values to magnetometer for hard iron calibration
  lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE); // Set magentometer performance mode to high
  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE); // Set magnetometer operation mode to continuous
  lis3mdl.setDataRate(LIS3MDL_DATARATE_560_HZ); // Set magnetometer data rate to 560 Hz
  lis3mdl.setRange(magRange); // Set magnetometer range to 4 gauss
  //lis3mdl.loadOffsetsFromEEPROM();  // Load offsets from EEPROM
  //lis3mdl.writeOffsetxyz();// Write offset values to magnetometer for hard iron calibration
  icm20649.setI2CBypass(false); // Disable I2C bypass

  
  icm20649.odrAlign(true); // Enable ODR alignment
  icm20649.setAccelRange(accelRange); // Set accelerometer range to 30 G
  icm20649.setGyroRange(gyroRange); // Set gyroscope range to 500 DPS
  icm20649.setAccelRateDivisor(getAHRSSampleRateDivisor()); // Set accelerometer data rate divisor
  icm20649.setGyroRateDivisor(getAHRSSampleRateDivisor()); // Set gyroscope data rate divisor 
  
  icm20649.enableI2CMaster(true); // Enable I2C master
  icm20649.configureI2CMaster(); // Configure I2C master
  icm20649.configI2CSlave0(LIS3MDL_I2CADDR_DEFAULT, LIS3MDL_REG_OUT_X_L, LIS3MDL_OUT_DATA_LEN); // Configure I2C slave 0
  
  icm20649.enableFIFO(true); // Enable FIFO
  icm20649.enableFIFOWatermarkInt(true, false); // Enable FIFO watermark interrupt
  icm20649.selectFIFOData(FIFO_DATA_ACCEL_GYRO_S0); // Select FIFO data
  icm20649.resetFIFO(); // Reset FIFO

  fusionFilter.begin(getAHRSSampleRate()); // Initialize fusion filter
  pinMode(AHRS_ITERRUPT_PIN,INPUT);//attach interrupt to pin 4
  attachInterrupt(digitalPinToInterrupt(AHRS_ITERRUPT_PIN), inputDataInterrupt, RISING); //attach interrupt to pin 4
  queue = xQueueCreate(1000, sizeof(ahrs_axes_t));//create queue for 500 elements of type ahrs_axes_t
  return true; // Success
}

/**
 * @brief Set the AHRS low power mode
 * 
 * This function sets the low power mode for the ICM20649 and LIS3MDL sensors.
 * (NON FUNCIONAL YET)
 * @param mode The low power mode to set (true for low power mode, false for normal mode)
 * @return bool Status of the low power mode configuration (true for success, false for failure)
*/
bool AHRS::lowPowerMode(bool mode)
{
  if (mode)
  {
    icm20649.enableI2CMaster(false);//disble i2c master
    icm20649.setI2CBypass(true);//enable bypass to acces lis2mdl
    //Serial.println("Check 1");
    lis3mdl.lowPowerMode(true);//enable lis3mdl low power mode
    //Serial.println("Check 2");
    icm20649.setI2CBypass(false);
    //Serial.println("Check 3");
    icm20649.sleepMode(true);//enable icm20049 sleep mode 
    //Serial.println("Check 4");
    icm20649.enableFIFO(false);//diable fifo transfers
    //Serial.println("Check 5");
    icm20649.resetFIFO();//reset fifo
  }else{
    icm20649.enableI2CMaster(false);//disble i2c master
    icm20649.setI2CBypass(true);//enable bypass to acces lis2mdl
    lis3mdl.lowPowerMode(false);//disable lis3mdl low power mode
    icm20649.setI2CBypass(false);
    icm20649.enableI2CMaster(true);//enable i2c master
    icm20649.sleepMode(false);//enable icm20049 sleep mode 
    delay(200); //delay for data stabilization 
    icm20649.resetFIFO();//reset fifo
    icm20649.enableFIFO(true);//enable fifo transfers
  }
  return true;
  
}

/**
 * @brief Set the AHRS sensor ranges
 * 
 * This function sets the accelerometer, gyroscope, and magnetometer ranges for the AHRS sensors.
 * 
 * @param accelRange The accelerometer range to set
 * @param gyroRange The gyroscope range to set
 * @param magRange The magnetometer range to set
 * @return bool Status of the range configuration (true for success, false for failure)
 */
bool AHRS::setAHRSRange(icm20649_accel_range_t accelRange, icm20649_gyro_range_t gyroRange, lis3mdl_range_t magRange) {
    icm20649.setAccelRange(accelRange);
    icm20649.setGyroRange(gyroRange);
    lis3mdl.setRange(magRange);
    return true;
}


/**
 * @breif Scale the axes data
 * 
 * This function scales the raw axes data from the ICM20649 and LIS3MDL sensors to their respective units based on the configured ranges
 * 
 * @param raw_axes The raw axes data to scale
 * @return ahrs_axes_t The scaled axes data
 */
ahrs_axes_t AHRS::scaleAxes(icm20x_raw_axes_t raw_axes){
    ahrs_axes_t scaled_axes;

    
    float accel_scale = 1.0;
    float gyro_scale = 1.0;
    float mag_scale = 1.0;
  
    if (gyroRange == ICM20649_GYRO_RANGE_500_DPS)
      gyro_scale = 65.5;
    if (gyroRange == ICM20649_GYRO_RANGE_1000_DPS)
      gyro_scale = 32.8;
    if (gyroRange == ICM20649_GYRO_RANGE_2000_DPS)
      gyro_scale = 16.4;
    if (gyroRange == ICM20649_GYRO_RANGE_4000_DPS)
      gyro_scale = 8.2;
  
    if (accelRange == ICM20649_ACCEL_RANGE_4_G)
      accel_scale = 8192.0;
    if (accelRange == ICM20649_ACCEL_RANGE_8_G)
      accel_scale = 4096.0;
    if (accelRange == ICM20649_ACCEL_RANGE_16_G)
      accel_scale = 2048.0;
    if (accelRange == ICM20649_ACCEL_RANGE_30_G)
      accel_scale = 1024.0;

    if (magRange == LIS3MDL_RANGE_4_GAUSS)
        mag_scale = 6842.0;
    if (magRange == LIS3MDL_RANGE_8_GAUSS)
        mag_scale = 3421.0;
    if (magRange == LIS3MDL_RANGE_12_GAUSS)
        mag_scale = 2281.0;
    if (magRange == LIS3MDL_RANGE_16_GAUSS)
        mag_scale = 1711.0;
  
    scaled_axes.accX = raw_axes.rawAccX / accel_scale;
    scaled_axes.accY = raw_axes.rawAccY / accel_scale;
    scaled_axes.accZ = raw_axes.rawAccZ / accel_scale;

    scaled_axes.gyroX = raw_axes.rawGyroX / gyro_scale;
    scaled_axes.gyroY = raw_axes.rawGyroY / gyro_scale;
    scaled_axes.gyroZ = raw_axes.rawGyroZ / gyro_scale;

    scaled_axes.magX = raw_axes.rawMagX / mag_scale;   
    scaled_axes.magY = raw_axes.rawMagY / mag_scale; 
    scaled_axes.magZ = raw_axes.rawMagZ / mag_scale;

    return scaled_axes;

}

/*!
 * @brief Correct the axis of the sensors to match in orientation and apply offsets
 *
 * @param scaled_axes The scaled axes data to correct
 * @return ahrs_axes_t The corrected axes data 
 */
ahrs_axes_t AHRS::correctAxex(ahrs_axes_t scaled_axes)
{
  ahrs_axes_t corrected_axes;
  corrected_axes.accX = -scaled_axes.accY;
  corrected_axes.accY = scaled_axes.accX;
  corrected_axes.accZ = scaled_axes.accZ;
  corrected_axes.gyroX = -scaled_axes.gyroY;
  corrected_axes.gyroY = scaled_axes.gyroX;
  corrected_axes.gyroZ = scaled_axes.gyroZ;
  corrected_axes.magX = scaled_axes.magY;
  corrected_axes.magY = -scaled_axes.magX;
  corrected_axes.magZ = scaled_axes.magZ;

  corrected_axes.gyroX = corrected_axes.gyroX - gyroOffsets.xoffset;
  corrected_axes.gyroY = corrected_axes.gyroY - gyroOffsets.yoffset;
  corrected_axes.gyroZ = corrected_axes.gyroZ - gyroOffsets.zoffset;
  corrected_axes.magX = corrected_axes.magX - magOffsets.xoffset;
  corrected_axes.magY = corrected_axes.magY - magOffsets.yoffset;
  corrected_axes.magZ = corrected_axes.magZ - magOffsets.zoffset;

  return corrected_axes;
}

/**
 * @brief Save magnetometer calibration offsets to EEPROM
 * 
 * This function saves the magnetometer hard iron calibration offsets to EEPROM.
 */
void AHRS::saveMagCalibToEEPROM()
{
float _x=0, _y=0, _z=0;
lis3mdl.getCalibrationOffsets(&_x, &_y, &_z);
EEPROM.put(MAG_X_OFFSET*sizeof(float), _x); // Save X offset to EEPROM
EEPROM.put(MAG_Y_OFFSET*sizeof(float), _y); // Save Y offset to EEPROM
EEPROM.put(MAG_Z_OFFSET*sizeof(float), _z); // Save Z offset to EEPROM
EEPROM.commit(); // Commit changes to EEPROM

}

/**
 * @brief Load magnetometer calibration offsets from EEPROM
 * 
 * This function loads the magnetometer hard iron calibration offsets from EEPROM.
 */
void AHRS::loadMagCalibFromEEPROM()
{
  float x = EEPROM.readFloat(MAG_X_OFFSET*sizeof(float));
  float y = EEPROM.readFloat(MAG_Y_OFFSET*sizeof(float));
  float z = EEPROM.readFloat(MAG_Z_OFFSET*sizeof(float));
  lis3mdl.setCalibrationOffsets(x, y, z); // Set calibration offsets to magnetometer
}

/**
 * @brief Call the hard iron calibration compute function for the magnetometer
 * 
 * @param x The X value for hard iron calibration
 * @param y The Y value for hard iron calibration
 * @param z The Z value for hard iron calibration
 */
void AHRS::magHardIronCalc(float x, float y, float z)
{
  //Serial.println("MagX: " + String(x) + " MagY: " + String(y) + " MagZ: " + String(z));
  lis3mdl.hardIronCalc(x, y, z); // Calculate hard iron offsets
}

/**
 * @brief Update the hard iron calibration compute function 
 * 
 * @param x The X value for hard iron calibration
 * @param y The Y value for hard iron calibration
 * @param z The Z value for hard iron calibration
 */
void AHRS::hardIronCalib(float x, float y, float z)
{
  static float maxX = 0, minX = 0, maxY = 0, minY = 0, maxZ = 0, minZ = 0;
  if (!magCalibrationFlag)
  {
    maxX = 0;
    minX = 0;
    maxY = 0;
    minY = 0;
    maxZ = 0;
    minZ = 0;
    magCalibrationFlag = true;
  }
  
  if (x > maxX) {
    maxX = x;
  } else if (x < minX) {
    minX = x;
  }
  if (y > maxY) {
    maxY = y;
  } else if (y < minY) {
    minY = y;
  }
  if (z > maxZ) {
    maxZ = z;
  } else if (z < minZ) {
    minZ = z;
  }
  this->magOffsetsTemp.xoffset = (maxX + minX) / 2.0;
  this->magOffsetsTemp.yoffset = (maxY + minY) / 2.0;
  this->magOffsetsTemp.zoffset = (maxZ + minZ) / 2.0;
  //Serial.println("MaxX: " + String(maxX) + " MinX: " + String(minX) + " MaxY: " + String(maxY) + " MinY: " + String(minY) + " MaxZ: " + String(maxZ) + " MinZ: " + String(minZ) + "-->Hard Iron x:" + String(this->magOffsetsTemp.xoffset) + " y:" + String(this->magOffsetsTemp.yoffset) + " z:" + String(this->magOffsetsTemp.zoffset));
}

/*!
  @brief Setter for the hard iron calibration values for external use
  
  @param x The X value for hard iron calibration
  @param y The Y value for hard iron calibration
  @param z The Z value for hard iron calibration
*/
void AHRS::setHardIronOffsets(float x, float y, float z)
{
  magOffsets.xoffset = x;
  magOffsets.yoffset = y;
  magOffsets.zoffset = z;
}

/*!
  @brief End calibration flag and transfer the values to the class attributes 
*/
void AHRS::endHardIronCalib()
{
  magCalibrationFlag = false;
  magOffsets.xoffset = magOffsetsTemp.xoffset;
  magOffsets.yoffset = magOffsetsTemp.yoffset;
  magOffsets.zoffset = magOffsetsTemp.zoffset;

}

/**
 * @brief Write the hard iron calibration offsets to the magnetometer
 * 
 * This function enables the  I2C bypass and call the writeOffsetxyz function to write the hard iron calibration offsets to the magnetometer.
 * 
 * @param x The X value for hard iron calibration
 * @param y The Y value for hard iron calibration
 * @param z The Z value for hard iron calibration
 */
void AHRS::magCalibWrite(){
  icm20649.enableI2CMaster(false);//disble i2c master
  icm20649.setI2CBypass(true);//enable bypass to acces lis2mdl
  lis3mdl.writeOffsetxyz();// Write offset values to magnetometer for hard iron calibration
  icm20649.setI2CBypass(false);
  icm20649.enableI2CMaster(true);//enable i2c master
}

/*!
  @brief Getter fot he  the AHRS sample rate 

  @return Sample rate 
*/
uint16_t AHRS::getAHRSSampleRate()
{
  switch (sampleRate)
  {
  case AHRS_15HZ:
    return 15;
    break;
  case AHRS_25HZ:
    return 25;
    break;
  case AHRS_45HZ:
    return 45;
    break;
  case AHRS_125HZ:
    return 125;
    break;
  case AHRS_225HZ:
    return 225;
    break;
  case AHRS_375HZ:
    return 375;
    break;
  case AHRS_1125HZ:
    return 1125;
    break;
  default:
    return 15;
    break;
  }
}

/*!
  @brief Getter for the AHRS sample rate divisor for ICM 20649 configuration

  @return Sample rate divisor 
*/
uint8_t AHRS::getAHRSSampleRateDivisor()
{
  switch (sampleRate)
  {
  case AHRS_15HZ:
    return 74;
    break;
  case AHRS_25HZ:
    return 44;
    break;
  case AHRS_45HZ:
    return 24;
    break;
  case AHRS_125HZ:
    return 8;
    break;
  case AHRS_225HZ:
    return 4;
    break;
  case AHRS_375HZ:
    return 2;
    break;
  case AHRS_1125HZ:
    return 0;
    break;
  default:
    return 74;
    break;
  }
}

/*!
  @brief Getter for the magnetometer offsets

  @return Magnetometer offsets
*/

offsets_t AHRS::getMagOffsets()
{
  return magOffsets;
}

/*!
  @brief Getter for the gyroscope offsets

  @return Gyroscope offsets
*/
offsets_t AHRS::getGyroOffsets()
{
  return gyroOffsets;
}

/*!
  @brief Update the Madgwick filter and get the orientation

  @param  scaled_axes Structure with the last AHRS measurements 
  @return Structure with roll, pitch and yaw angles.
*/
ahrs_orientation_t AHRS::computeAHRSOrientation(ahrs_axes_t axes)
{
  ahrs_orientation_t orientation = {0, 0, 0};

  fusionFilter.update(
    axes.gyroX,
    axes.gyroY,
    axes.gyroZ,
    axes.accX,
    axes.accY,
    axes.accZ,
    axes.magX*100.f,//convert to uTeslas and invert X y axis to correct orientation
    axes.magY*100.f,
    axes.magZ*100.f
  );
  orientation.roll = fusionFilter.getRoll();
  orientation.pitch = fusionFilter.getPitch();
  orientation.yaw = fusionFilter.getYaw();
  return orientation;
}

/*!
  @brief Update the Madgwick filter and get the inclination and direction angles

  @param  scaled_axes Structure with the last AHRS measurements 
  @return Structure with inclination and direction angles.
*/
ahrs_angles_t AHRS::computeAHRSInclination(ahrs_axes_t scaled_axes)
{
  ahrs_angles_t angles = {0, 0, {0, 0, 0}};

  fusionFilter.update(
    -scaled_axes.gyroY,
    scaled_axes.gyroX,
    scaled_axes.gyroZ,
    -scaled_axes.accY,
    scaled_axes.accX,
    scaled_axes.accZ,
    scaled_axes.magY*100.f,//convert to uTeslas and invert X y axis to correct orientation
    -scaled_axes.magX*100.f,
    scaled_axes.magZ*100.f
  );
  
  angles.inclination = fusionFilter.getInclination();
  angles.orientation.roll = fusionFilter.getRoll();
  angles.orientation.pitch = fusionFilter.getPitch();
  angles.orientation.yaw = fusionFilter.getYaw();
  return angles;
}

bool AHRS::ahrsUpdate(ahrs_axes_t *axes, ahrs_orientation_t *orientation, bool *shockDetected)
{
  if (inputDataAvailable) {
    inputDataAvailable = false;//clear the flag
    //unsigned long t0 = millis(); // Start time for data processing
    uint16_t frameCount = icm20649.readFIFOBuffer(raw_axesD);// Read FIFO buffer
    //Serial.println("Time to read FIFO: " + String(millis() - t0) + "ms"); // Debugging time taken to read FIFO
    for (size_t i = 0; i < frameCount; i++) { // Process each frame of data
      ahrs_axes_t scaled_axes = scaleAxes(raw_axesD[i]);
      ahrs_axes_t corrected_axes = correctAxex(scaled_axes);
      if (xQueueSend(queue, &corrected_axes, 0) != pdPASS) {
        #if(DEBUG_AHRS)
            Serial.println("[AHRS] Queue full");
        #endif
      }
    }
  }

  if (xQueueReceive(queue, axes, 0) == pdPASS) {
    *orientation = computeAHRSOrientation(*axes);
    *shockDetected = shockCheck(*axes); // Check for shock events
    return true; // data available
  }
  return false; // no data available
}


/*!
    * @brief Check for shock events
    * 
    * This function checks for shock events based on the accelerometer data.
    * 
    * @param dataFrame Data frame containing accelerometer data
    * @return true if a shock event is detected, false otherwise
*/
bool AHRS::shockCheck(ahrs_axes_t dataFrame) {
    // Shift older values in the buffer
    memmove(dataFrameBuffer, dataFrameBuffer + 1, sizeof(ahrs_axes_t) * (SHOCK_BUFFER_LENGTH - 1));

    // Insert new data frame into the last position of the buffer
    dataFrameBuffer[(SHOCK_BUFFER_LENGTH - 1)] = dataFrame;
  
    // Calculate the average of the last 3 samples
    ahrs_axes_t avgSample;
    avgSample.accX = (dataFrameBuffer[0].accX + dataFrameBuffer[1].accX + dataFrameBuffer[2].accX) / 3.f;
    avgSample.accY = (dataFrameBuffer[0].accY + dataFrameBuffer[1].accY + dataFrameBuffer[2].accY) / 3.f;
    avgSample.accZ = (dataFrameBuffer[0].accZ + dataFrameBuffer[1].accZ + dataFrameBuffer[2].accZ) / 3.f;
  
    // Calculate the derivative between the new sample and the average of the last 3 samples
    float accXDiff = (dataFrameBuffer[(SHOCK_BUFFER_LENGTH - 1)].accX - avgSample.accX);
    float accYDiff = (dataFrameBuffer[(SHOCK_BUFFER_LENGTH - 1)].accY - avgSample.accY);
    float accZDiff = (dataFrameBuffer[(SHOCK_BUFFER_LENGTH - 1)].accZ - avgSample.accZ);
  
    // Check for abrupt acceleration
    if (abs(accXDiff) > shockThreshold || abs(accYDiff) > shockThreshold || abs(accZDiff) > shockThreshold) {
        #if(DEBUG_AHRS)
            Serial.println("[DataLogger]");
            Serial.println("SHOCK! in time:" + String(millis()));
            Serial.print("Average X: " + String(avgSample.accX) );
            Serial.print("Average Y: " + String(avgSample.accY) );
            Serial.println("Average Z: " + String(avgSample.accZ) );
            Serial.print("New X: " + String(dataFrameBuffer[3].accX) );
            Serial.print("New Y: " + String(dataFrameBuffer[3].accY) );
            Serial.println("New Z: " + String(dataFrameBuffer[3].accZ) );
            Serial.print("Diff X: " + String(accXDiff) );
            Serial.print("Diff Y: " + String(accYDiff) );
            Serial.println("Diff Z: " + String(accZDiff) );
            Serial.println("END SHOCK!");
            Serial.println("[DataLogger]");
        #endif
        return true;
    } else {
        return false;
    }
  }


