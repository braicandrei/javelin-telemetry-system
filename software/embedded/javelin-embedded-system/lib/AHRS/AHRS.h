/*!
    * @file AHRS.h
    * Attitude and Heading Reference System (AHRS) class
*/

#ifndef AHRS_H
#define AHRS_H  

#include <Adafruit_ICM20649.h>
#include <Adafruit_LIS3MDL.h>

typedef struct {
    float accX, ///< Accelerometer X axis in G's
    accY,      ///< Accelerometer Y axis in G's
    accZ,      ///< Accelerometer Z axis in G's
    gyroX,     ///< Gyroscope X axis in DPS
    gyroY,     ///< Gyroscope Y axis in DPS
    gyroZ,     ///< Gyroscope Z axis in DPS
    magX,      ///< Magnetometer X axis in uT
    magY,      ///< Magnetometer Y axis in uT
    magZ;      ///< Magnetometer Z axis in uT
} ahrs_axes_t;

class AHRS {    
public:
    AHRS();
    ~AHRS();
    uint8_t beginAHRSi2c();
    bool configAHRS();
    bool lowPowerMode(bool mode);
    bool setAHRSRange(
        icm20649_accel_range_t accelRange, icm20649_gyro_range_t gyroRange, lis3mdl_range_t magRange);
    bool setAHRSSampleRate(uint8_t sampleRate);
    ahrs_axes_t scaleAxes(icm20x_raw_axes_t raw_axes);
    Adafruit_ICM20649 icm20649;
    Adafruit_LIS3MDL lis3mdl;
private:
protected:
icm20x_raw_axes_t raw_axes[ICM20X_FIFO_SIZE/6];  ///< Raw data axes buffer array (set to max size of frames in FIFO)
icm20649_accel_range_t accelRange = ICM20649_ACCEL_RANGE_30_G; ///< Accelerometer range
icm20649_gyro_range_t gyroRange = ICM20649_GYRO_RANGE_500_DPS; ///< Gyroscope range
lis3mdl_range_t magRange =LIS3MDL_RANGE_4_GAUSS; ///< Magnetometer range

};


#endif