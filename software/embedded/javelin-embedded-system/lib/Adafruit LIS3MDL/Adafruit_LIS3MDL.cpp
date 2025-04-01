/*!
 * @file     Adafruit_LIS3MDL.cpp
 *
 * @mainpage Adafruit LIS3MDL Breakout
 *
 * @section intro_sec Introduction
 *
 * This is a library for the Adafruit LIS3MDL magnetometer breakout board
 * ----> https://www.adafruit.com/product/4479
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @author   Limor Fried (Adafruit Industries)
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Arduino.h"
#include <Adafruit_LIS3MDL.h>
#include <Wire.h>

/**************************************************************************/
/*!
    @brief  Instantiates a new LIS3MDL class
*/
/**************************************************************************/
Adafruit_LIS3MDL::Adafruit_LIS3MDL()
 {
 }

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_LIS3MDL::begin_I2C(uint8_t i2c_address, TwoWire *wire) {
  if (i2c_dev)
    delete i2c_dev;
  if (spi_dev)
    delete spi_dev;

  spi_dev = NULL;
  i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

  if (!i2c_dev->begin()) {
    return false;
  }
  Serial.println("i2c_dev->begin() CHECK");
  return _init();
}

/*!
 *    @brief  Sets up the hardware and initializes hardware SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  theSPI The SPI object to be used for SPI connections.
 *    @param  frequency The SPI bus frequency
 *    @return True if initialization was successful, otherwise false.
 */
boolean Adafruit_LIS3MDL::begin_SPI(uint8_t cs_pin, SPIClass *theSPI,
                                    uint32_t frequency) {
  if (i2c_dev)
    delete i2c_dev;
  if (spi_dev)
    delete spi_dev;

  i2c_dev = NULL;
  spi_dev = new Adafruit_SPIDevice(cs_pin,
                                   frequency,             // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0,             // data mode
                                   theSPI);
  if (!spi_dev->begin()) {
    return false;
  }
  return _init();
}

/*!
 *    @brief  Sets up the hardware and initializes software SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  sck_pin The arduino pin # connected to SPI clock
 *    @param  miso_pin The arduino pin # connected to SPI MISO
 *    @param  mosi_pin The arduino pin # connected to SPI MOSI
 *    @param  frequency The SPI bus frequency
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_LIS3MDL::begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                                 int8_t mosi_pin, uint32_t frequency) {

  if (i2c_dev)
    delete i2c_dev;
  if (spi_dev)
    delete spi_dev;

  i2c_dev = NULL;
  spi_dev = new Adafruit_SPIDevice(cs_pin, sck_pin, miso_pin, mosi_pin,
                                   frequency,             // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0);            // data mode

  if (!spi_dev->begin()) {
    return false;
  }
  return _init();
}

/*!
 *    @brief  Common initialization code for I2C & SPI
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_LIS3MDL::_init(void) {
  // Check connection
  Adafruit_BusIO_Register chip_id =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_WHO_AM_I, 1);

  // make sure we're talking to the right chip
  if (chip_id.read() != 0x3D) {
    // No LIS3MDL detected ... return false
    return false;
  }

  reset();

  // set high quality performance mode
  setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);

  // 155Hz default rate
  setDataRate(LIS3MDL_DATARATE_155_HZ);

  // lowest range
  setRange(LIS3MDL_RANGE_4_GAUSS);

  setOperationMode(LIS3MDL_CONTINUOUSMODE);

  return true;
}

/**************************************************************************/
/*!

@brief  Performs a software reset
*/
/**************************************************************************/
void Adafruit_LIS3MDL::reset(void) {
  Adafruit_BusIO_Register CTRL_REG2 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG2, 1);
  Adafruit_BusIO_RegisterBits resetbits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG2, 1, 2);
  resetbits.write(0x1);
  delay(10);

  getRange();
}

/**************************************************************************/
/*!
  @brief  Read the XYZ data from the magnetometer and store in the internal
  x, y and z (and x_g, y_g, z_g) member variables.
*/
/**************************************************************************/

void Adafruit_LIS3MDL::read(void) {
  uint8_t buffer[6];

  Adafruit_BusIO_Register XYZDataReg = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC, LIS3MDL_REG_OUT_X_L, 6);
  XYZDataReg.read(buffer, 6);
  x = buffer[0];
  x |= buffer[1] << 8;
  y = buffer[2];
  y |= buffer[3] << 8;
  z = buffer[4];
  z |= buffer[5] << 8;

  float scale = 1; // LSB per gauss
  switch (rangeBuffered) {
  case LIS3MDL_RANGE_16_GAUSS:
    scale = 1711;
    break;
  case LIS3MDL_RANGE_12_GAUSS:
    scale = 2281;
    break;
  case LIS3MDL_RANGE_8_GAUSS:
    scale = 3421;
    break;
  case LIS3MDL_RANGE_4_GAUSS:
    scale = 6842;
    break;
  }

  x_gauss = (float)x / scale;
  y_gauss = (float)y / scale;
  z_gauss = (float)z / scale;
}

/**************************************************************************/
/*!
    @brief  Gets the most recent sensor event, Adafruit Unified Sensor format
    @param  event Pointer to an Adafruit Unified sensor_event_t object that
   we'll fill in
    @returns True on successful read
*/
/**************************************************************************/
bool Adafruit_LIS3MDL::getEvent(sensors_event_t *event) {
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_MAGNETIC_FIELD;
  event->timestamp = millis();

  read();

  event->magnetic.x = x_gauss * 100; // microTesla per gauss
  event->magnetic.y = y_gauss * 100; // microTesla per gauss
  event->magnetic.z = z_gauss * 100; // microTesla per gauss

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t device data, Adafruit Unified Sensor format
    @param  sensor Pointer to an Adafruit Unified sensor_t object that we'll
   fill in
*/
/**************************************************************************/
void Adafruit_LIS3MDL::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "LIS3MDL", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_MAGNETIC_FIELD;
  sensor->min_delay = 0;
  sensor->min_value = -1600;  // -16 gauss in uTesla
  sensor->max_value = 1600;   // +16 gauss in uTesla
  sensor->resolution = 0.015; // 100/6842 uTesla per LSB at +-4 gauss range
}

/**************************************************************************/
/*!
    @brief Set the performance mode, LIS3MDL_LOWPOWERMODE, LIS3MDL_MEDIUMMODE,
    LIS3MDL_HIGHMODE or LIS3MDL_ULTRAHIGHMODE
    @param mode Enumerated lis3mdl_performancemode_t
*/
/**************************************************************************/
void Adafruit_LIS3MDL::setPerformanceMode(lis3mdl_performancemode_t mode) {
  // write xy
  Adafruit_BusIO_Register CTRL_REG1 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG1, 1);
  Adafruit_BusIO_RegisterBits performancemodebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG1, 2, 5);
  performancemodebits.write((uint8_t)mode);

  // write z
  Adafruit_BusIO_Register CTRL_REG4 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG4, 1);
  Adafruit_BusIO_RegisterBits performancemodezbits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG4, 2, 2);
  performancemodezbits.write((uint8_t)mode);
}

/**************************************************************************/
/*!
    @brief Get the performance mode
    @returns Enumerated lis3mdl_performancemode_t, LIS3MDL_LOWPOWERMODE,
    LIS3MDL_MEDIUMMODE, LIS3MDL_HIGHMODE or LIS3MDL_ULTRAHIGHMODE
*/
/**************************************************************************/
lis3mdl_performancemode_t Adafruit_LIS3MDL::getPerformanceMode(void) {
  Adafruit_BusIO_Register CTRL_REG1 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG1, 1);
  Adafruit_BusIO_RegisterBits performancemodebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG1, 2, 5);
  return (lis3mdl_performancemode_t)performancemodebits.read();
}

/**************************************************************************/
/*!
    @brief  Sets the data rate for the LIS3MDL (controls power consumption)
    from 0.625 Hz to 80Hz
    @param dataRate Enumerated lis3mdl_dataRate_t
*/
/**************************************************************************/
void Adafruit_LIS3MDL::setDataRate(lis3mdl_dataRate_t dataRate) {
  if (dataRate == LIS3MDL_DATARATE_155_HZ) {
    // set OP to UHP
    setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);
  }
  if (dataRate == LIS3MDL_DATARATE_300_HZ) {
    // set OP to HP
    setPerformanceMode(LIS3MDL_HIGHMODE);
  }
  if (dataRate == LIS3MDL_DATARATE_560_HZ) {
    // set OP to MP
    setPerformanceMode(LIS3MDL_MEDIUMMODE);
  }
  if (dataRate == LIS3MDL_DATARATE_1000_HZ) {
    // set OP to LP
    setPerformanceMode(LIS3MDL_LOWPOWERMODE);
  }
  delay(10);
  Adafruit_BusIO_Register CTRL_REG1 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG1, 1);
  Adafruit_BusIO_RegisterBits dataratebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG1, 4, 1); // includes FAST_ODR
  dataratebits.write((uint8_t)dataRate);
}

/**************************************************************************/
/*!
    @brief  Gets the data rate for the LIS3MDL (controls power consumption)
    @return Enumerated lis3mdl_dataRate_t from 0.625 Hz to 80Hz
*/
/**************************************************************************/
lis3mdl_dataRate_t Adafruit_LIS3MDL::getDataRate(void) {
  Adafruit_BusIO_Register CTRL_REG1 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG1, 1);
  Adafruit_BusIO_RegisterBits dataratebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG1, 4, 1); // includes FAST_ODR
  return (lis3mdl_dataRate_t)dataratebits.read();
}

/**************************************************************************/
/*!
    @brief Set the operation mode, LIS3MDL_CONTINUOUSMODE,
    LIS3MDL_SINGLEMODE or LIS3MDL_POWERDOWNMODE
    @param mode Enumerated lis3mdl_operationmode_t
*/
/**************************************************************************/
void Adafruit_LIS3MDL::setOperationMode(lis3mdl_operationmode_t mode) {
  // write x and y
  Adafruit_BusIO_Register CTRL_REG3 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG3, 1);
  Adafruit_BusIO_RegisterBits opmodebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG3, 2, 0);
  opmodebits.write((uint8_t)mode);
}

/**************************************************************************/
/*!
    @brief Get the operation mode
    @returns Enumerated lis3mdl_operationmode_t, LIS3MDL_CONTINUOUSMODE,
    LIS3MDL_SINGLEMODE or LIS3MDL_POWERDOWNMODE
*/
/**************************************************************************/
lis3mdl_operationmode_t Adafruit_LIS3MDL::getOperationMode(void) {
  Adafruit_BusIO_Register CTRL_REG3 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG3, 1);
  Adafruit_BusIO_RegisterBits opmodebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG3, 2, 0);
  return (lis3mdl_operationmode_t)opmodebits.read();
}

/**************************************************************************/
/*!
    @brief Set the resolution range: +-4 gauss, 8 gauss, 12 gauss, or 16 gauss.
    @param range Enumerated lis3mdl_range_t
*/
/**************************************************************************/
void Adafruit_LIS3MDL::setRange(lis3mdl_range_t range) {
  Adafruit_BusIO_Register CTRL_REG2 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG2, 1);
  Adafruit_BusIO_RegisterBits rangebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG2, 2, 5);
  rangebits.write((uint8_t)range);

  rangeBuffered = range;
  
  this->_scale = 1.0;
  switch (rangeBuffered) {
  case LIS3MDL_RANGE_16_GAUSS:
    this->_scale = 1711.0;
    break;
  case LIS3MDL_RANGE_12_GAUSS:
    this->_scale = 2281.0;
    break;
  case LIS3MDL_RANGE_8_GAUSS:
    this->_scale = 3421.0;
    break;
  case LIS3MDL_RANGE_4_GAUSS:
    this->_scale = 6842.0;
    break;
  }
}

/**************************************************************************/
/*!
    @brief Read the resolution range: +-4 gauss, 8 gauss, 12 gauss, or 16 gauss.
    @returns Enumerated lis3mdl_range_t
*/
/**************************************************************************/
lis3mdl_range_t Adafruit_LIS3MDL::getRange(void) {
  Adafruit_BusIO_Register CTRL_REG2 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG2, 1);
  Adafruit_BusIO_RegisterBits rangebits =
      Adafruit_BusIO_RegisterBits(&CTRL_REG2, 2, 5);

  rangeBuffered = (lis3mdl_range_t)rangebits.read();

  return rangeBuffered;
}

/**************************************************************************/
/*!
    @brief Set the interrupt threshold value
    @param value 16-bit unsigned raw value
*/
/**************************************************************************/
void Adafruit_LIS3MDL::setIntThreshold(uint16_t value) {
  value &= 0x7FFF; // high bit must be 0!
  Adafruit_BusIO_Register INT_THS =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_INT_THS_L, 2);
  INT_THS.write(value);
}

/**************************************************************************/
/*!
    @brief Get the interrupt threshold value
    @returns 16-bit unsigned raw value
*/
/**************************************************************************/
uint16_t Adafruit_LIS3MDL::getIntThreshold(void) {
  Adafruit_BusIO_Register INT_THS =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_INT_THS_L, 2);
  return INT_THS.read();
}

/**************************************************************************/
/*!
    @brief Configure INT_CFG
    @param enableX Enable interrupt generation on X-axis
    @param enableY Enable interrupt generation on Y-axis
    @param enableZ Enable interrupt generation on Z-axis
    @param polarity Sets the polarity of the INT output logic
    @param latch If true (latched) the INT pin remains in the same state
    until INT_SRC is read.
    @param enableInt Interrupt enable on INT pin
*/
/**************************************************************************/
void Adafruit_LIS3MDL::configInterrupt(bool enableX, bool enableY, bool enableZ,
                                       bool polarity, bool latch,
                                       bool enableInt) {
  uint8_t value = 0x08; // set default bits, see table 36
  value |= enableX << 7;
  value |= enableY << 6;
  value |= enableZ << 5;
  value |= polarity << 2;
  value |= latch << 1;
  value |= enableInt;

  Adafruit_BusIO_Register INT_CFG = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC, LIS3MDL_REG_INT_CFG, 1);
  INT_CFG.write(value);
}

/**************************************************************************/
/*!
    @brief Enable or disable self-test
    @param flag If true, enable self-test

*/
/**************************************************************************/
void Adafruit_LIS3MDL::selfTest(bool flag) {
  Adafruit_BusIO_Register CTRL_REG1 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG1, 1);

  Adafruit_BusIO_RegisterBits stbit =
      Adafruit_BusIO_RegisterBits(&CTRL_REG1, 1, 0);

  stbit.write(flag);
}

/**************************************************************************/
/*!
    @brief Get the magnetic data rate.
    @returns The data rate in float
*/
float Adafruit_LIS3MDL::magneticFieldSampleRate(void) {
  switch (this->getDataRate()) {
  case LIS3MDL_DATARATE_0_625_HZ:
    return 0.625f;
  case LIS3MDL_DATARATE_1_25_HZ:
    return 1.25f;
  case LIS3MDL_DATARATE_2_5_HZ:
    return 2.5f;
  case LIS3MDL_DATARATE_5_HZ:
    return 5.0f;
  case LIS3MDL_DATARATE_10_HZ:
    return 10.0f;
  case LIS3MDL_DATARATE_20_HZ:
    return 20.0f;
  case LIS3MDL_DATARATE_40_HZ:
    return 40.0f;
  case LIS3MDL_DATARATE_80_HZ:
    return 80.0f;
  case LIS3MDL_DATARATE_155_HZ:
    return 155.0f;
  case LIS3MDL_DATARATE_300_HZ:
    return 300.0f;
  case LIS3MDL_DATARATE_560_HZ:
    return 560.0f;
  case LIS3MDL_DATARATE_1000_HZ:
    return 1000.0f;
  }

  return 0;
}

/**************************************************************************/
/*!
    @brief Check for available data from magnetic
    @returns 1 if available, 0 if not
*/
int Adafruit_LIS3MDL::magneticFieldAvailable(void) {
  Adafruit_BusIO_Register REG_STATUS = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC, LIS3MDL_REG_STATUS, 1);
  return (REG_STATUS.read() & 0x08) ? 1 : 0;
}

/**************************************************************************/
/*!
    @brief Read magnetic data
    @param x reference to x axis
    @param y reference to y axis
    @param z reference to z axis
    @returns 1 if success, 0 if not
*/
int Adafruit_LIS3MDL::readMagneticField(float &x, float &y, float &z) {
  int16_t data[3];

  Adafruit_BusIO_Register XYZDataReg = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC, LIS3MDL_REG_OUT_X_L, 6);

  if (!XYZDataReg.read((uint8_t *)data, sizeof(data))) {
    x = y = z = NAN;
    return 0;
  }

  x = data[0] * 4.0 * 100.0 / 32768.0;
  y = data[1] * 4.0 * 100.0 / 32768.0;
  z = data[2] * 4.0 * 100.0 / 32768.0;

  return 1;
}

bool Adafruit_LIS3MDL::resetRegisters() {
  Adafruit_BusIO_Register CTRL_REG2 =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_REG_CTRL_REG2, 1);
      Adafruit_BusIO_RegisterBits CTRL_REG2_rst =
      Adafruit_BusIO_RegisterBits(&CTRL_REG2, 1, 2);
  CTRL_REG2_rst.write(true); // reset all registers to default values
  delay(20);
  while (CTRL_REG2_rst.read()) {
    delay(10);
  };
  delay(50);
  return true;
}

bool Adafruit_LIS3MDL::writeOffsetxyz() {

  int16_t xOffsetInt = (int16_t)(this->xoffset * this->_scale);
  int16_t yOffsetInt = (int16_t)(this->yoffset * this->_scale);
  int16_t zOffsetInt = (int16_t)(this->zoffset * this->_scale);

  Adafruit_BusIO_Register OFFSET_X_REG_L =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_OFFSET_X_REG_L, 6);
  uint8_t buffer[6];
  buffer[0] = xOffsetInt & 0xFF;
  buffer[1] = (xOffsetInt >> 8) & 0xFF;
  buffer[2] = yOffsetInt & 0xFF;
  buffer[3] = (yOffsetInt >> 8) & 0xFF;
  buffer[4] = zOffsetInt & 0xFF;
  buffer[5] = (zOffsetInt >> 8) & 0xFF;

  return OFFSET_X_REG_L.write(buffer, sizeof(buffer));
} 

bool Adafruit_LIS3MDL::hardIronCalib(float *x, float *y, float *z, uint16_t size) {
  //calculate max and min values of x, y and z (array of floats passed by reference)
  float maxX = x[0], minX = x[0], maxY = y[0], minY = y[0], maxZ = z[0], minZ = z[0];
  float xoffsetNew = 0, yoffsetNew = 0, zoffsetNew = 0;
  for (uint16_t i = 1; i < size; i++) {
    if (x[i] > maxX) {
      maxX = x[i];
    } else if (x[i] < minX) {
      minX = x[i];
    }
    if (y[i] > maxY) {
      maxY = y[i];
    } else if (y[i] < minY) {
      minY = y[i];
    }
    if (z[i] > maxZ) {
      maxZ = z[i];
    } else if (z[i] < minZ) {
      minZ = z[i];
    }
  }
  //calculate offsets

  xoffsetNew = (maxX + minX) / 2.0;
  yoffsetNew = (maxY + minY) / 2.0;
  zoffsetNew = (maxZ + minZ) / 2.0;

  Serial.print("xoffset compute: ");
  Serial.println(xoffsetNew);
  Serial.print("yoffset compute: ");
  Serial.println(yoffsetNew);
  Serial.print("zoffset compute: ");
  Serial.println(zoffsetNew);

  this->xoffset += xoffsetNew;
  this->yoffset += yoffsetNew;
  this->zoffset += zoffsetNew;
  saveOffsetsToEEPROM();
  return writeOffsetxyz();
}

void Adafruit_LIS3MDL::getCalibrationOffsets(float *x, float *y, float *z) {
  *x = this->xoffset;
  *y = this->yoffset;
  *z = this->zoffset;
}

void Adafruit_LIS3MDL::readCalibrationOffsets(float *x, float *y, float *z) {
//read calibration offsets from registers and convert based on scale
  int16_t xOffsetInt, yOffsetInt, zOffsetInt;
  Adafruit_BusIO_Register OFFSET_X_REG_L =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, AD8_HIGH_TOREAD_AD7_HIGH_TOINC,
                              LIS3MDL_OFFSET_X_REG_L, 6);
  uint8_t buffer[6];
  OFFSET_X_REG_L.read(buffer, sizeof(buffer));
  xOffsetInt = (int16_t)(buffer[0] | (buffer[1] << 8));
  yOffsetInt = (int16_t)(buffer[2] | (buffer[3] << 8));
  zOffsetInt = (int16_t)(buffer[4] | (buffer[5] << 8));
  
  *x = (float)xOffsetInt / this->_scale;
  *y = (float)yOffsetInt / this->_scale;
  *z = (float)zOffsetInt / this->_scale;
}


void Adafruit_LIS3MDL::loadOffsetsFromEEPROM() {
  float xOffset, yOffset, zOffset;

  // Leer los valores almacenados en la EEPROM
  EEPROM.get(EEPROM_X_OFFSET_ADDR, xOffset);
  EEPROM.get(EEPROM_Y_OFFSET_ADDR, yOffset);
  EEPROM.get(EEPROM_Z_OFFSET_ADDR, zOffset);

  // Asignar los valores leídos a los atributos de la clase
  this->xoffset = xOffset;
  this->yoffset = yOffset;
  this->zoffset = zOffset;

  Serial.print("xoffset load: ");
  Serial.println(this->xoffset);
  Serial.print("yoffset load: ");
  Serial.println(this->yoffset);
  Serial.print("zoffset load: ");
  Serial.println(this->zoffset);
}

// Método para guardar los offsets en la EEPROM después de la calibración
void Adafruit_LIS3MDL::saveOffsetsToEEPROM() {
  // Guardar los valores actuales de los offsets en la EEPROM
  EEPROM.put(EEPROM_X_OFFSET_ADDR, this->xoffset);
  EEPROM.put(EEPROM_Y_OFFSET_ADDR, this->yoffset);
  EEPROM.put(EEPROM_Z_OFFSET_ADDR, this->zoffset);
  //EEPROM.put(EEPROM_X_OFFSET_ADDR,0.f);
  //EEPROM.put(EEPROM_Y_OFFSET_ADDR, 0.f);
  //EEPROM.put(EEPROM_Z_OFFSET_ADDR, 0.f);
  EEPROM.commit(); //
}