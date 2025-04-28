/*!
    * @file AHRS.h
    * Attitude and Heading Reference System (AHRS) class
*/

#ifndef AHRS_H
#define AHRS_H  

#include <Adafruit_ICM20649.h>
#include <Adafruit_LIS3MDL.h>
#include <MadgwickAHRS.h>

#define EEPROM_SIZE 1024 ///< Size of EEPROM for storing system parameters
enum EEPROM_ADDR {
    MAG_X_OFFSET, ///< Magnetometer X axis offset address
    MAG_Y_OFFSET, ///< Magnetometer Y axis offset address
    MAG_Z_OFFSET, ///< Magnetometer Z axis offset address
};


typedef struct {
    float xoffset,
    yoffset,
    zoffset;
} mag_offsets_t;

typedef struct {
    float accX, ///< Accelerometer X axis in G's
    accY,      ///< Accelerometer Y axis in G's
    accZ,      ///< Accelerometer Z axis in G's
    gyroX,     ///< Gyroscope X axis in DPS
    gyroY,     ///< Gyroscope Y axis in DPS
    gyroZ,     ///< Gyroscope Z axis in DPS
    magX,      ///< Magnetometer X axis in gauss
    magY,      ///< Magnetometer Y axis in gauss
    magZ;      ///< Magnetometer Z axis in gauss
} ahrs_axes_t;

typedef struct {
    float roll, pitch, yaw; 
} ahrs_orientation_t;

typedef struct {
    float direction, inclination;
} ahrs_angles_t;

typedef enum
{
    AHRS_WAITING,
    AHRS_PREP,
    AHRS_SAMPLING,
    AHRS_END
}state_t;

typedef enum
{   
    AHRS_15HZ,
    AHRS_25HZ,
    AHRS_45HZ,
    AHRS_125HZ,
    AHRS_225HZ,
    AHRS_375HZ,
    AHRS_1125HZ
} ahrs_sample_rate_t;

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

    void magHardIronCalc(float x, float y, float z);
    void hardIronCalib(float x, float y, float z);
    void setHardIronOffsets(float x, float y, float z);
    void endHardIronCalib();
    void magCalibWrite();
    void saveMagCalibToEEPROM();
    void loadMagCalibFromEEPROM();
    ahrs_orientation_t computeAHRSOrientation(ahrs_axes_t scaled_axes);
    ahrs_angles_t computeAHRSAngles(ahrs_axes_t scaled_axes);
    void calibrateRelativeOrientation(ahrs_orientation_t newOrientation);
    ahrs_orientation_t relativeOrientation = {0, 0, 0};
    Adafruit_ICM20649 icm20649;
    Adafruit_LIS3MDL lis3mdl;
private:
    bool magCalibrationFlag = false;
    mag_offsets_t magOffsetsTemp = {0, 0, 0};
    mag_offsets_t magOffsets = {-0.04, 0.08, -0.47};
protected:
icm20x_raw_axes_t raw_axes[ICM20X_FIFO_SIZE/6];  ///< Raw data axes buffer array (set to max size of frames in FIFO)
icm20649_accel_range_t accelRange = ICM20649_ACCEL_RANGE_30_G; ///< Accelerometer range
icm20649_gyro_range_t gyroRange = ICM20649_GYRO_RANGE_500_DPS; ///< Gyroscope range
lis3mdl_range_t magRange =LIS3MDL_RANGE_4_GAUSS; ///< Magnetometer range

ahrs_sample_rate_t sampleRate = AHRS_225HZ; ///< AHRS sample rate
uint16_t getAHRSSampleRate();
uint8_t getAHRSSampleRateDivisor();

Madgwick fusionFilter;

};


#endif