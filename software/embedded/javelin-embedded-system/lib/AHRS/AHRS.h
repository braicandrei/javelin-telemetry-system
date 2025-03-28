/*!
    * @file AHRS.h
    * Attitude and Heading Reference System (AHRS) class
*/

#ifndef AHRS_H
#define AHRS_H  

#include <Adafruit_ICM20649.h>
#include <Adafruit_LIS3MDL.h>


class AHRS {    
public:
    AHRS();
    ~AHRS();
    uint8_t beginAHRS( );
    bool setAHRSRange(
        icm20649_accel_range_t accelRange, icm20649_gyro_range_t gyroRange, lis3mdl_range_t magRange);
    bool configAHRS();
    bool setAHRSSampleRate(uint8_t sampleRate);

    Adafruit_ICM20649 icm20649;
    Adafruit_LIS3MDL lis3mdl;
private:

protected:
icm20x_raw_axes_t raw_axes[ICM20X_FIFO_SIZE/6];  ///< Raw data axes buffer array (set to max size of frames in FIFO)


};


#endif