/********************************************************************
 *
 * Module Name : ICM-20948.cpp
 *
 * Author/Date : C.B. Lirakis / 24-Feb-22
 *
 * Modified   BY   Reason
 * --------   --   ------
 * 15-Dec-23  CBL  stopped using wiring-pi
 *
 * Description : Generic ICM-20948
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 * 
 * https://www.sparkfun.com/products/15335
 * https://learn.sparkfun.com/tutorials/raspberry-pi-spi-and-i2c-tutorial#i2c-on-pi
 * http://wiringpi.com
 * http://wiringpi.com/reference/i2c-library/
 * https://github.com/drcpattison/ICM-20948/  - USED A LOT OF CODE FROM THIS
 * https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface
 * This is particularly helpful:
 * https://github.com/danjperron/A2D_PIC_RPI/blob/master/I2CWrapper.c
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <limits.h>

// Local Includes.
#include "debug.h"
#include "CLogger.hh"
#include "ICM-20948.hh"
#include "I2CHelper.hh"

/**
 ******************************************************************
 *
 * Function Name : ICM20948 constructor
 *
 * Description :
 *     Set default scale values for Gyro, Acceleromter and Magnetic mode
 *     Based on those defaults set the scaling factors. 
 *     Initialize the ICM-20948 chip and open the I2C channel
 *     Initialize the AK09916 chip and open the I2C channel
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : 
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
ICM20948::ICM20948 (uint8_t IMU_address) : CObject()
{
    SET_DEBUG_STACK;
    SetName("ICM20948");
    ClearError(__LINE__);
    SetUniqueID(1);

    fIMUAddress = IMU_address;


    /*
     * Lets use this in setup FIXME. 
     */
    fGscale = GFS_250DPS;
    fAscale = AFS_2G;

    fGres = getGres();
    fAres = getAres();
    fIMU_address = IMU_address;

    if(!InitICM20948())
    {
	SetError(-1, __LINE__);
	return;
    }

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : ICM20948 destructor
 *
 * Description : PLACEHOLDER
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
ICM20948::~ICM20948 (void)
{
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  InitICM20948
 *
 * Description :
 *       setup the unit and initialize to the desired modes of operation
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::InitICM20948(void)
{
    SET_DEBUG_STACK;
    CLogger   *log  = CLogger::GetThis();
    I2CHelper *pI2C = I2CHelper::GetThis();
    ClearError(__LINE__);

    if (fIMU_address <= 0)
    {
	log->Log("# IMU subsystem off.\n");
	return true;
    }
    log->LogTime(" Attempting to attach to I2C network. \n");


    /*
     * According to the datasheet the power management (CLKSEL[2:0])
     * has to be set to 001 to achieve the performance in the datasheet.
     * Also look at power modes in 4.22
     */
    int rv = pI2C->WriteReg8( fIMU_address, PWR_MGMT_1, 0x01);  
  
    /*
     *  Switch to user bank 2 - registers for setup
     * see section 14.8 in the user documention.
     * see also 7.3 (0x7F) 
     * more detail in section 8
     * 8.65 - bits 5:4 select the bank, here it is bank 2
     */
    rv = pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    /*
     * Configure Gyro and Thermometer
     * Disable FSYNC and set gyro bandwidth to 51.2 Hz, respectively;
     * minimum delay time for this setting is 5.9 ms, which means sensor 
     * fusion update rates cannot be higher than 1 / 0.0059 = 170 Hz
     * DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz 
     * for both With the ICM20948, it is possible to get gyro sample rates 
     * of 32 kHz (!), 8 kHz, or 1 kHz
     * Set gyroscope full scale range to 250 dps
     * GYRO_CONFIG_1 setup Section:10.2
     *    Bit   Function
     *     0    1-Enable/0-Disable Gyro DLPF
     *    2:1   Full scale +/-
     *          00    250 dps
     *          01    500 dps
     *          10   1000 dps
     *          11   2000 dps
     *    5:3   Low pass filter configuation. Table 16
     *
     * TEMP_CONFIG
     *     bits(2:0) - Low pass filter
     *        0        7932 Hz   9KHz
     *        1         217.9    1.125
     *        2         123.5    1.125
     *        3          65.9    1.125
     *        4          34.1    1.125
     *        5          17.3    1.125
     *        6           8.8    ???
     *        7         7932     9
     *
     */
    rv = pI2C->WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x19); 
    rv = pI2C->WriteReg8( fIMU_address, TEMP_CONFIG, 0x03);

    /*
     * Set sample rate = gyroscope output rate(1.1kHz)/(1 + GYRO_SMPLRT_DIV)
     * 
     * Use a 220 Hz rate; a rate consistent with the filter update rate
     * determined inset in CONFIG above.
     * 
     * Gyro sample rate divider Section 10.1
     *  
     */
    rv = pI2C->WriteReg8( fIMU_address, GYRO_SMPLRT_DIV, 0x04);

    /*
     *
     * Set gyroscope full scale range
     * Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are
     * left-shifted into positions 4:3
     *
     * Set accelerometer full-scale range configuration
     * Get current ACCEL_CONFIG register value (Section 10.15)
     *
     *  Bits
     *  7:6     Reserved
     *  5:3     Accelerometer low pass filter see table 18
     *  2:1     Full scale +/-
     *          00  2g
     *          01  4g
     *          10  8g
     *          11 16g
     *   0      1 Enable, 0 Disable DLPF
     */
    uint8_t c = pI2C->ReadReg8( fIMU_address, ACCEL_CONFIG);

    // c = c & ~0xE0;    // Clear self-test bits [7:5]
    c = c & ~0x06;       // Clear AFS bits [4:3]
    c = c | fAscale<<1;  // Set full scale range for the accelerometer
    c = c | 0x01;        // Set enable accel DLPF for the accelerometer
    c = c | 0x18;        // and set DLFPFCFG to 50.4 hz (table 18)

    // Write new ACCEL_CONFIG register value
    rv = pI2C->WriteReg8( fIMU_address, ACCEL_CONFIG, c);

    /*
     * Set accelerometer sample rate configuration
     * It is possible to get a 4 kHz sample rate from the accelerometer by
     * choosing 1 for accel_fchoice_b bit [3]; in this case the bandwidth is
     * 1.13 kHz
     * 
     * Section 10.12 (LSB for sample rate) 1.125kHz/(1+ACCEL_SSMPLRT_DIV[11:0]
     */
    rv = pI2C->WriteReg8( fIMU_address, ACCEL_SMPLRT_DIV_2, 0x04);

    /*
     * Do we neet to set MSB too - lets be complete, CBL addition
     */
    rv = pI2C->WriteReg8( fIMU_address, ACCEL_SMPLRT_DIV_1, 0x0);

    /*
     * The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
     * but all these rates are further reduced by a factor of 5 to 200 Hz 
     * because of the GYRO_SMPLRT_DIV setting
     *
     * Switch to user bank 0 to access this. Section 7.1
     * Bank 0 is also where all the registers for reading data are. 
     */
    rv = pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);

    /*
     *
     * Configure Interrupts and Bypass Enable
     * Set interrupt pin active high, push-pull, hold interrupt pin level HIGH
     * until interrupt cleared, clear on read of INT_STATUS, and enable
     * I2C_BYPASS_EN so additional chips can join the I2C bus and all can be
     * controlled by the Arduino as master.
     * 
     * INT_PIN_CFG Section 8.6
     *   BIT
     *    7    (1) INT 1 active low, (0) active high
     *    6    (1) INT1 pin is open drain, (0) Push-Pull
     *    5    (1) INT1 pin level until cleared, (0) pulse only 50us in length
     *    4    (1) Interrupt status cleared on any read, (0) only on read of 
     *         INT_STATUS 
     *    3    ACTL Set - FSYNC pin active low
     *    2    FSYNC Enable/Disable
     *    1    Bypass Enabled
     *    0    Reserved
     *
     *    0x22 --- Hold interrupt until cleared, Bypass enabled
     */
    rv = pI2C->WriteReg8( fIMU_address, INT_PIN_CFG, 0x22);

    // Enable data ready (bit 0) interrupt
    rv = pI2C->WriteReg8( fIMU_address, INT_ENABLE_1, 0x01);

    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : getGres
 *
 * Description : Return the resolution of the gyro depending
 *               on the setup. 
 *
 * Inputs : NONE
 *
 * Returns : Gryo resolution in degrees/second
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 26-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
double ICM20948::getGres(void)
{
    SET_DEBUG_STACK;
    double gRes;

    switch (fGscale)
    {
	/*
	 * Possible gyro scales (and their register bit settings) are:
	 * 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS (11).
	 * Here's a bit of an algorith to calculate DPS/(ADC tick) based 
	 * on that 2-bit value:
	 */
    case GFS_250DPS:
	gRes = 250.0/32768.0;
	break;
    case GFS_500DPS:
	gRes = 500.0/32768.0;
	break;
    case GFS_1000DPS:
	gRes = 1000.0/32768.0;
	break;
    case GFS_2000DPS:
	gRes = 2000.0/32768.0;
	break;
    }
    return gRes;
}

/**
 ******************************************************************
 *
 * Function Name : getAres
 *
 * Description : base on the scaling setting return the scaling value
 *
 * Inputs : NONE
 *
 * Returns : Scaling value for accelerometer
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 26-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
double ICM20948::getAres(void)
{
    SET_DEBUG_STACK;
    double aRes = 0.0;

    switch (fAscale)
    {
	// Possible accelerometer scales (and their register bit settings) are:
	// 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
	// Here's a bit of an algorith to calculate DPS/(ADC tick) based on 
	// that 2-bit value:
    case AFS_2G:
	aRes = 2.0/32768.0;
	break;
    case AFS_4G:
	aRes = 4.0/32768.0;
	break;
    case AFS_8G:
	aRes = 8.0f/32768.0;
	break;
    case AFS_16G:
	aRes = 16.0/32768.0;
	break;
    }
    return aRes;
}
/**
 ******************************************************************
 *
 * Function Name : readTempData
 *
 * Description : 
 *     reads the temperature data directly from the register. 
 *
 * Inputs : NONE
 *
 * Returns : Temperature in degrees C according to the scale outlined
 *           in the datasheet.
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
double ICM20948::readTempData(void)
{
    SET_DEBUG_STACK;
    const double RoomTemp_Offset  = 40.0;    // lower end of temperature scale
    const double Temp_Sensitivity = 333.87;  // LSB/C
    I2CHelper *pI2C = I2CHelper::GetThis();

    double   Temp = 0.0;
    uint8_t  Hi, Lo;
    uint16_t itemp;

    // Read out temperature sensor data Hi order byte
    //Hi = wiringPiI2CReadReg8(fIMU_address, TEMP_OUT_H);
    // Read out temperature sensor data Low order byte
    //Lo = wiringPiI2CReadReg8(fIMU_address, TEMP_OUT_L);

    // Turn the MSB and LSB into a 16-bit value
    Hi = pI2C->ReadReg8( fIMU_address, TEMP_OUT_H);
    Lo = pI2C->ReadReg8( fIMU_address,TEMP_OUT_L);
    itemp = ((int16_t)Hi << 8) | Lo;

    /*
     * Temp C = ((TempOut - RoomTemp_Offset)/Temp_Sensitivity)+21.0
     * I think RoomTemp_Offset is wrong. 
     */
    Temp = ((double)itemp - RoomTemp_Offset)/Temp_Sensitivity + 21.0;
#if 0
    cout << " Hi: " << (int) Hi
	 << " Lo: " << (int) Lo
	 << " itemp: " << itemp
	 << " ftemp: " << fTemp
	 << endl;
#endif

    SET_DEBUG_STACK;
    return Temp;
}

/**
 ******************************************************************
 *
 * Function Name : readAccelData
 *
 * Description :
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 26-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::readAccelData(double *XYZ)
{
    SET_DEBUG_STACK;
    I2CHelper *pI2C = I2CHelper::GetThis();

    uint8_t *ptr;
    ptr = (uint8_t *)&itemp;

    // Read out sensor data Hi order byte
    *(ptr+1) = pI2C->ReadReg8(fIMU_address, ACCEL_XOUT_H);
    // Read out sensor data Low order byte
    *ptr = pI2C->ReadReg8(fIMU_address, ACCEL_XOUT_L);

    XYZ[0] = (double)itemp * fAres;

    // Read out sensor data Hi order byte
    *(ptr+1) = pI2C->ReadReg8(fIMU_address, ACCEL_YOUT_H);
    // Read out sensor data Low order byte
    *ptr = pI2C->ReadReg8(fIMU_address, ACCEL_YOUT_L);

    XYZ[1] = (double)itemp * fAres;

    // Read out sensor data Hi order byte
    *(ptr+1) = pI2C->ReadReg8(fIMU_address, ACCEL_ZOUT_H);
    // Read out sensor data Low order byte
    *ptr = pI2C->ReadReg8(fIMU_address, ACCEL_ZOUT_L);

    XYZ[2] = (double)itemp * fAres;

    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : readGyroData
 *
 * Description :
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 26-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::readGyroData(double *XYZ)
{
    SET_DEBUG_STACK;
    I2CHelper *pI2C = I2CHelper::GetThis();

    uint8_t *ptr;
    ptr = (uint8_t *)&itemp;

    // Read out sensor data Hi order byte
    *(ptr+1) = pI2C->ReadReg8(fIMU_address, GYRO_XOUT_H);
    // Read out sensor data Low order byte
    *ptr = pI2C->ReadReg8(fIMU_address, GYRO_XOUT_L);

    XYZ[0] = (double)itemp * fGres;

    // Read out sensor data Hi order byte
    *(ptr+1) = pI2C->ReadReg8(fIMU_address, GYRO_YOUT_H);
    // Read out sensor data Low order byte
    *ptr = pI2C->ReadReg8(fIMU_address, GYRO_YOUT_L);

    XYZ[1] = (double)itemp * fGres;

    // Read out sensor data Hi order byte
    *(ptr+1) = pI2C->ReadReg8(fIMU_address, GYRO_ZOUT_H);
    // Read out sensor data Low order byte
    *ptr = pI2C->ReadReg8(fIMU_address, GYRO_ZOUT_L);

    XYZ[2] = (double)itemp * fGres;

    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : ICM20948SelfTest
 *
 * Description :
 *     Accelerometer and gyroscope self test; check calibration wrt 
 *     factory settings should return percent deviation from factory 
 *     trim values, +/- 14 or less deviation is a pass.
 * 
 *     Self test is a run then exit function. 
 *     
 * Inputs : 
 *     destination - double array of at least 6 values that returns
 *     
 *
 * Returns : true on success
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::ICM20948SelfTest(double * destination)
{
    const struct timespec sleeptime = {0L, 25000000L};
    const int32_t kNAVG = 200;
    const uint8_t FS    = 0;         // What is this? 
    const double scale  = (double)(2620/1<<FS);
    SET_DEBUG_STACK;
    I2CHelper *pI2C = I2CHelper::GetThis();
    CLogger   *log  = CLogger::GetThis();

    uint8_t selfTest[6];
    double  Acc[3], Gyro[3];
    double  aAvg[3], gAvg[3];
    double  aSTAvg[3];
    double  gSTAvg[3];
    double  factoryTrim[6];

    if (!destination)
    {
	SET_DEBUG_STACK;
	return false;
    }

    log->Log("# Performing self tests. \n");

    memset(aAvg,   0, 3*sizeof(double));
    memset(gAvg,   0, 3*sizeof(double));
    memset(aSTAvg, 0, 3*sizeof(double));
    memset(gSTAvg, 0, 3*sizeof(double));

    // Get stable time source
    // Auto select clock source to be PLL gyroscope reference if ready else
    pI2C->WriteReg8( fIMU_address,  PWR_MGMT_1, 0x01);
  
    // Switch to user bank 2
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    // Set gyro sample rate to 1 kHz
    pI2C->WriteReg8( fIMU_address, GYRO_SMPLRT_DIV, 0x00);

    // Set gyro sample rate to 1 kHz, DLPF to 119.5 Hz and FSR to 250 dps
    pI2C->WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x11);

    // Set accelerometer rate to 1 kHz and bandwidth to 111.4 Hz
    // Set full scale range for the accelerometer to 2 g
    pI2C->WriteReg8( fIMU_address, ACCEL_CONFIG, 0x11);

    // Switch to user bank 0
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);

    // Hang out a bit, wait for the chip to stablize. 
    nanosleep(&sleeptime, NULL);

    // Get average current values of gyro and acclerometer
    for (int ii = 0; ii < kNAVG; ii++)
    {
	log->Log("# BHW::ii = %d\n", ii);

	// Read The acclerometer data and average it. 
	readAccelData(Acc);
	aAvg[0] += Acc[0];
	aAvg[1] += Acc[1];
	aAvg[2] += Acc[2];

	// Read Gyro data. 
	readGyroData(Gyro);
	gAvg[0] += Gyro[0];
	gAvg[1] += Gyro[1];
	gAvg[2] += Gyro[2];
    }

    // Get average of 200 values and store as average current readings
    for (int ii =0; ii < 3; ii++)
    {
	aAvg[ii] /= ((double)kNAVG);
	gAvg[ii] /= ((double)kNAVG);
    }
  
    // Switch to user bank 2
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    /*
     * Configure the accelerometer for self-test
     * Enable self test on all three axes and set accelerometer 
     * range to +/- 2 g
     */
    pI2C->WriteReg8( fIMU_address, ACCEL_CONFIG_2, 0x1C);

    /*
     * Enable self test on all three axes and set gyro range 
     * to +/- 250 degrees/s
     */
    pI2C->WriteReg8( fIMU_address, GYRO_CONFIG_2,  0x38);

  
    // Switch to user bank 0
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);

    // Hang out a bit, wait for the chip to stablize. 
    nanosleep(&sleeptime, NULL);

    // Get average self-test values of gyro and acclerometer
    for (int ii = 0; ii < kNAVG; ii++)
    {
	readAccelData(Acc);
	aSTAvg[0] += Acc[0];
	aSTAvg[1] += Acc[1];
	aSTAvg[2] += Acc[2];

	// Read Gyro data. 
	readGyroData(Gyro);
	gSTAvg[0] += Gyro[0];
	gSTAvg[1] += Gyro[1];
	gSTAvg[2] += Gyro[2];
    }

    // Get average of 200 values and store as average self-test readings
    for (int ii =0; ii < 3; ii++)
    {
	aSTAvg[ii] /= ((double)kNAVG);
	gSTAvg[ii] /= ((double)kNAVG);
    }
  
    // Switch to user bank 2
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    // Configure the gyro and accelerometer for normal operation
    pI2C->WriteReg8( fIMU_address, ACCEL_CONFIG_2, 0x00);
    pI2C->WriteReg8( fIMU_address, GYRO_CONFIG_2,  0x00);
    
  
    // Switch to user bank 1
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x10);

    /*
     * Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
     * X-axis accel self-test results
     */
    selfTest[0] = pI2C->ReadReg8( fIMU_address, SELF_TEST_X_ACCEL);
    // Y-axis accel self-test results
    selfTest[1] = pI2C->ReadReg8( fIMU_address, SELF_TEST_Y_ACCEL);
    // Z-axis accel self-test results
    selfTest[2] = pI2C->ReadReg8( fIMU_address, SELF_TEST_Z_ACCEL);
    // X-axis gyro self-test results
    selfTest[3] = pI2C->ReadReg8( fIMU_address, SELF_TEST_X_GYRO);
    // Y-axis gyro self-test results
    selfTest[4] = pI2C->ReadReg8( fIMU_address, SELF_TEST_Y_GYRO);
    // Z-axis gyro self-test results
    selfTest[5] = pI2C->ReadReg8( fIMU_address, SELF_TEST_Z_GYRO);
  
    // Switch to user bank 0
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);
    // Hang out a bit, wait for the chip to stablize. 
    nanosleep(&sleeptime, NULL);


    /*
     * Retrieve factory self-test value from self-test code reads
     * FT[Xa] factory trim calculation
     */
    factoryTrim[0] = scale*(pow(1.01,((double)selfTest[0] - 1.0)));
    // FT[Ya] factory trim calculation
    factoryTrim[1] = scale*(pow(1.01,((double)selfTest[1] - 1.0) ));
    // FT[Za] factory trim calculation
    factoryTrim[2] = scale*(pow(1.01,((double)selfTest[2] - 1.0) ));
    // FT[Xg] factory trim calculation
    factoryTrim[3] = scale*(pow(1.01,((double)selfTest[3] - 1.0) ));
    // FT[Yg] factory trim calculation
    factoryTrim[4] = scale*(pow(1.01,((double)selfTest[4] - 1.0) ));
    // FT[Zg] factory trim calculation
    factoryTrim[5] = scale*(pow(1.01,((double)selfTest[5] - 1.0) ));

    /*
     * Report results as a ratio of (STR - FT)/FT; the change from Factory Trim
     * of the Self-Test Response
     * To get percent, must multiply by 100
     * Why subtract 100?
     */
    for (int i = 0; i < 3; i++)
    {
	// Report percent differences
	destination[i]   = 100.0*((aSTAvg[i]-aAvg[i]))/factoryTrim[i] - 100.;
	// Report percent differences
	destination[i+3] = 100.0*((gSTAvg[i]-gAvg[i]))/factoryTrim[i+3] - 100.;
    }

    log->Log("# Self tests complete. \n");
    log->Log("#    acc: %f %f %f \n", 
	     destination[0], destination[1], destination[2]);
    log->Log("#    gyro: %f %f %f \n", 
	     destination[3], destination[4], destination[5]);

    return true;
}

#if 0

// Function which accumulates gyro and accelerometer data after device
// initialization. It calculates the average of the at-rest readings and then
// loads the resulting offsets into accelerometer and gyro bias registers.
void ICM20948::calibrateICM20948(float * gyroBias, float * accelBias)
{
    uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
    uint16_t ii, packet_count, fifo_count;
    int32_t gyro_bias[3]  = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

	
    // reset device
    // Write a one to bit 7 reset bit; toggle reset device
    pI2C->WriteReg8( fIMU_address, PWR_MGMT_1, READ_FLAG);
    delay(200);

    // get stable time source; Auto select clock source to be PLL gyroscope
    // reference if ready else use the internal oscillator, bits 2:0 = 001
    pI2C->WriteReg8( fIMU_address, PWR_MGMT_1, 0x01);
    delay(200);

    // Configure device for bias calculation
    // Disable all interrupts
    pI2C->WriteReg8( fIMU_address, INT_ENABLE, 0x00);
    // Disable FIFO
    pI2C->WriteReg8( fIMU_address, FIFO_EN_1, 0x00);
    pI2C->WriteReg8( fIMU_address, FIFO_EN_2, 0x00);
    // Turn on internal clock source
    pI2C->WriteReg8( fIMU_address, PWR_MGMT_1, 0x00);
    // Disable I2C master
    //pI2C->WriteReg8( fIMU_address, I2C_MST_CTRL, 0x00); Already disabled
    // Disable FIFO and I2C master modes
    pI2C->WriteReg8( fIMU_address, USER_CTRL, 0x00);
    // Reset FIFO and DMP
    pI2C->WriteReg8( fIMU_address, USER_CTRL, 0x08);
    pI2C->WriteReg8( fIMU_address, FIFO_RST, 0x1F);
    delay(10);
    pI2C->WriteReg8( fIMU_address, FIFO_RST, 0x00);
    delay(15);

    // Set FIFO mode to snapshot
    pI2C->WriteReg8( fIMU_address, FIFO_MODE, 0x1F);
    // Switch to user bank 2
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);
    // Configure ICM20948 gyro and accelerometer for bias calculation
    // Set low-pass filter to 188 Hz
    pI2C->WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x01);
    // Set sample rate to 1 kHz
    pI2C->WriteReg8( fIMU_address, GYRO_SMPLRT_DIV, 0x00);
    // Set gyro full-scale to 250 degrees per second, maximum sensitivity
    pI2C->WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x00);
    // Set accelerometer full-scale to 2 g, maximum sensitivity
    pI2C->WriteReg8( fIMU_address, ACCEL_CONFIG, 0x00);

    uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
    uint16_t  accelsensitivity = 16384; // = 16384 LSB/g

    // Switch to user bank 0
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);
    // Configure FIFO to capture accelerometer and gyro data for bias calculation
    pI2C->WriteReg8( fIMU_address, USER_CTRL, 0x40);  // Enable FIFO
    // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in
    // ICM20948)
    pI2C->WriteReg8( fIMU_address, FIFO_EN_2, 0x1E);
    delay(40);  // accumulate 40 samples in 40 milliseconds = 480 bytes

    // At end of sample accumulation, turn off FIFO sensor read
    // Disable gyro and accelerometer sensors for FIFO
    pI2C->WriteReg8( fIMU_address, FIFO_EN_2, 0x00);
    // Read FIFO sample count
    readBytes(ICM20948_ADDRESS, FIFO_COUNTH, 2, &data[0]);
    fifo_count = ((uint16_t)data[0] << 8) | data[1];
    // How many sets of full gyro and accelerometer data for averaging
    packet_count = fifo_count/12;

    for (ii = 0; ii < packet_count; ii++)
    {
	int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
	// Read data for averaging
	readBytes(ICM20948_ADDRESS, FIFO_R_W, 12, &data[0]);
	// Form signed 16-bit integer for each sample in FIFO
	accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  );
	accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  );
	accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  );
	gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  );
	gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  );
	gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]);

	// Sum individual signed 16-bit biases to get accumulated signed 32-bit
	// biases.
	accel_bias[0] += (int32_t) accel_temp[0];
	accel_bias[1] += (int32_t) accel_temp[1];
	accel_bias[2] += (int32_t) accel_temp[2];
	gyro_bias[0]  += (int32_t) gyro_temp[0];
	gyro_bias[1]  += (int32_t) gyro_temp[1];
	gyro_bias[2]  += (int32_t) gyro_temp[2];
    }
    // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias[0] /= (int32_t) packet_count;
    accel_bias[1] /= (int32_t) packet_count;
    accel_bias[2] /= (int32_t) packet_count;
    gyro_bias[0]  /= (int32_t) packet_count;
    gyro_bias[1]  /= (int32_t) packet_count;
    gyro_bias[2]  /= (int32_t) packet_count;

    // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    if (accel_bias[2] > 0L)
    {
	accel_bias[2] -= (int32_t) accelsensitivity;
    }
    else
    {
	accel_bias[2] += (int32_t) accelsensitivity;
    }

    // Construct the gyro biases for push to the hardware gyro bias registers,
    // which are reset to zero upon device startup.
    // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input
    // format.
    data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF;
    // Biases are additive, so change sign on calculated average gyro biases
    data[1] = (-gyro_bias[0]/4)       & 0xFF;
    data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
    data[3] = (-gyro_bias[1]/4)       & 0xFF;
    data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
    data[5] = (-gyro_bias[2]/4)       & 0xFF;
  
    // Switch to user bank 2
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    // Push gyro biases to hardware registers
    pI2C->WriteReg8( fIMU_address, XG_OFFSET_H, data[0]);
    pI2C->WriteReg8( fIMU_address, XG_OFFSET_L, data[1]);
    pI2C->WriteReg8( fIMU_address, YG_OFFSET_H, data[2]);
    pI2C->WriteReg8( fIMU_address, YG_OFFSET_L, data[3]);
    pI2C->WriteReg8( fIMU_address, ZG_OFFSET_H, data[4]);
    pI2C->WriteReg8( fIMU_address, ZG_OFFSET_L, data[5]);

    // Output scaled gyro biases for display in the main program
    gyroBias[0] = (float) gyro_bias[0]/(float) gyrosensitivity;
    gyroBias[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
    gyroBias[2] = (float) gyro_bias[2]/(float) gyrosensitivity;

    // Construct the accelerometer biases for push to the hardware accelerometer
    // bias registers. These registers contain factory trim values which must be
    // added to the calculated accelerometer biases; on boot up these registers
    // will hold non-zero values. In addition, bit 0 of the lower byte must be
    // preserved since it is used for temperature compensation calculations.
    // Accelerometer bias registers expect bias input as 2048 LSB per g, so that
    // the accelerometer biases calculated above must be divided by 8.
  
    // Switch to user bank 1
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x10);
    // A place to hold the factory accelerometer trim biases
    int32_t accel_bias_reg[3] = {0, 0, 0};
    // Read factory accelerometer trim values
    readBytes(ICM20948_ADDRESS, XA_OFFSET_H, 2, &data[0]);
    accel_bias_reg[0] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
    readBytes(ICM20948_ADDRESS, YA_OFFSET_H, 2, &data[0]);
    accel_bias_reg[1] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
    readBytes(ICM20948_ADDRESS, ZA_OFFSET_H, 2, &data[0]);
    accel_bias_reg[2] = (int32_t) (((int16_t)data[0] << 8) | data[1]);

    // Define mask for temperature compensation bit 0 of lower byte of
    // accelerometer bias registers
    uint32_t mask = 1uL;
    // Define array to hold mask bit for each accelerometer bias axis
    uint8_t mask_bit[3] = {0, 0, 0};

    for (ii = 0; ii < 3; ii++)
    {
	// If temperature compensation bit is set, record that fact in mask_bit
	if ((accel_bias_reg[ii] & mask))
	{
	    mask_bit[ii] = 0x01;
	}
    }

    // Construct total accelerometer bias, including calculated average
    // accelerometer bias from above
    // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g
    // (16 g full scale)
    accel_bias_reg[0] -= (accel_bias[0]/8);
    accel_bias_reg[1] -= (accel_bias[1]/8);
    accel_bias_reg[2] -= (accel_bias[2]/8);

    data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
    data[1] = (accel_bias_reg[0])      & 0xFF;
    // preserve temperature compensation bit when writing back to accelerometer
    // bias registers
    data[1] = data[1] | mask_bit[0];
    data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
    data[3] = (accel_bias_reg[1])      & 0xFF;
    // Preserve temperature compensation bit when writing back to accelerometer
    // bias registers
    data[3] = data[3] | mask_bit[1];
    data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
    data[5] = (accel_bias_reg[2])      & 0xFF;
    // Preserve temperature compensation bit when writing back to accelerometer
    // bias registers
    data[5] = data[5] | mask_bit[2];

    // Apparently this is not working for the acceleration biases in the ICM-20948
    // Are we handling the temperature correction bit properly?
    // Push accelerometer biases to hardware registers
    pI2C->WriteReg8( fIMU_address, XA_OFFSET_H, data[0]);
    pI2C->WriteReg8( fIMU_address, XA_OFFSET_L, data[1]);
    pI2C->WriteReg8( fIMU_address, YA_OFFSET_H, data[2]);
    pI2C->WriteReg8( fIMU_address, YA_OFFSET_L, data[3]);
    pI2C->WriteReg8( fIMU_address, ZA_OFFSET_H, data[4]);
    pI2C->WriteReg8( fIMU_address, ZA_OFFSET_L, data[5]);

    // Output scaled accelerometer biases for display in the main program
    accelBias[0] = (float)accel_bias[0]/(float)accelsensitivity;
    accelBias[1] = (float)accel_bias[1]/(float)accelsensitivity;
    accelBias[2] = (float)accel_bias[2]/(float)accelsensitivity;
    // Switch to user bank 0
    pI2C->WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);
}

#endif
