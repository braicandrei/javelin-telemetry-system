/*!
    * @file AHRS.cpp
    * @brief AHRS class definition
    * @date 2021-02-21
*/

#include "AHRS.h"

AHRS::AHRS() {}
AHRS::~AHRS() {}

uint8_t AHRS::beginAHRS() {

    if (!icm20649.begin_I2C()) { // Initialize ICM20649 I2C bus
        return 0x01;
    }

    icm20649.setI2CBypass(true); // Enable I2C bypass

    if (!lis3mdl.begin_I2C()) { // Initialize LIS3MDL I2C bus
        //return 0x02;
    }
    lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE); // Set magentometer performance mode to high
    lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE); // Set magnetometer operation mode to continuous
    lis3mdl.setDataRate(LIS3MDL_DATARATE_560_HZ); // Set magnetometer data rate to 560 Hz
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS); // Set magnetometer range to 4 gauss
//
    icm20649.setI2CBypass(false); // Disable I2C bypass
    icm20649.setAccelRange(ICM20649_ACCEL_RANGE_30_G); // Set accelerometer range to 30 G
    icm20649.setGyroRange(ICM20649_GYRO_RANGE_4000_DPS); // Set gyroscope range to 500 DPS
    icm20649.setAccelRateDivisor(254); // Set accelerometer data rate divisor to 254
    icm20649.setGyroRateDivisor(254); // Set gyroscope data rate divisor to 254
    

    icm20649.enableI2CMaster(true); // Enable I2C master
    icm20649.configureI2CMaster(); // Configure I2C master
    icm20649.configI2CSlave0(LIS3MDL_I2CADDR_DEFAULT, LIS3MDL_REG_OUT_X_L, LIS3MDL_OUT_DATA_LEN); // Configure I2C slave 0

    icm20649.enableFIFO(true); // Enable FIFO
    
    icm20649.enableFIFOWatermarkInt(true, false); // Enable FIFO watermark interrupt
    
    icm20649.selectFIFOData(FIFO_DATA_ACCEL_GYRO_S0); // Select FIFO data
    
    icm20649.resetFIFO(); // Reset FIFO


    return 0x00;
}

bool AHRS::configAHRS() {

}

bool AHRS::setAHRSRange(icm20649_accel_range_t accelRange, icm20649_gyro_range_t gyroRange, lis3mdl_range_t magRange) {
    icm20649.setAccelRange(accelRange);
    icm20649.setGyroRange(gyroRange);
    lis3mdl.setRange(magRange);
    return true;
}


