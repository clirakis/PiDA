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

// specific to talking to I2C
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

// Local Includes.
#include "debug.h"
#include "CLogger.hh"
#include "ICM-20948.hh"

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
ICM20948::ICM20948 (uint8_t IMU_address, uint8_t Mag_address) : CObject(), IMUData()
{
    SET_DEBUG_STACK;
    SetName("ICM20948");
    ClearError(__LINE__);
    SetUniqueID(1);

    time_t     t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);

    /*
     * Lets use this in setup FIXME. 
     */
    fGscale = GFS_250DPS;
    fAscale = AFS_2G;
    fMmode  = M_100HZ;

    fGres = getGres();
    fAres = getAres();
    fIMU_address = IMU_address;
    fMag_address = Mag_address;

    fGMTOffset = lt.tm_gmtoff;
    //cout << "GMT OFFSET: " << fGMTOffset << endl;

    // Open the chip. 
    fdI2C = open(kICMDeviceName, O_RDWR);
    if (fdI2C<0)
    {
	SetError(-1, __LINE__);
	return;
    }

    if(!InitICM20948())
    {
	return;
    }

    /*
     * Second argument is the mode to acquire data. 
     */
    if(!InitAK09916(0x08))
    {
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
    close(fdI2C);
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
    CLogger *log = CLogger::GetThis();
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
    int rv = WriteReg8( fIMU_address, PWR_MGMT_1, 0x01);  
  
    /*
     *  Switch to user bank 2 - registers for setup
     * see section 14.8 in the user documention.
     * see also 7.3 (0x7F) 
     * more detail in section 8
     * 8.65 - bits 5:4 select the bank, here it is bank 2
     */
    rv = WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

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
    rv = WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x19); 
    rv = WriteReg8( fIMU_address, TEMP_CONFIG, 0x03);

    /*
     * Set sample rate = gyroscope output rate(1.1kHz)/(1 + GYRO_SMPLRT_DIV)
     * 
     * Use a 220 Hz rate; a rate consistent with the filter update rate
     * determined inset in CONFIG above.
     * 
     * Gyro sample rate divider Section 10.1
     *  
     */
    rv = WriteReg8( fIMU_address, GYRO_SMPLRT_DIV, 0x04);

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
    uint8_t c = ReadReg8( fIMU_address, ACCEL_CONFIG);

    // c = c & ~0xE0;    // Clear self-test bits [7:5]
    c = c & ~0x06;       // Clear AFS bits [4:3]
    c = c | fAscale<<1;  // Set full scale range for the accelerometer
    c = c | 0x01;        // Set enable accel DLPF for the accelerometer
    c = c | 0x18;        // and set DLFPFCFG to 50.4 hz (table 18)

    // Write new ACCEL_CONFIG register value
    rv = WriteReg8( fIMU_address, ACCEL_CONFIG, c);

    /*
     * Set accelerometer sample rate configuration
     * It is possible to get a 4 kHz sample rate from the accelerometer by
     * choosing 1 for accel_fchoice_b bit [3]; in this case the bandwidth is
     * 1.13 kHz
     * 
     * Section 10.12 (LSB for sample rate) 1.125kHz/(1+ACCEL_SSMPLRT_DIV[11:0]
     */
    rv = WriteReg8( fIMU_address, ACCEL_SMPLRT_DIV_2, 0x04);

    /*
     * Do we neet to set MSB too - lets be complete, CBL addition
     */
    rv = WriteReg8( fIMU_address, ACCEL_SMPLRT_DIV_1, 0x0);

    /*
     * The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
     * but all these rates are further reduced by a factor of 5 to 200 Hz 
     * because of the GYRO_SMPLRT_DIV setting
     *
     * Switch to user bank 0 to access this. Section 7.1
     * Bank 0 is also where all the registers for reading data are. 
     */
    rv = WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);

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
    rv = WriteReg8( fIMU_address, INT_PIN_CFG, 0x22);

    // Enable data ready (bit 0) interrupt
    rv = WriteReg8( fIMU_address, INT_ENABLE_1, 0x01);

    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : InitAK09916
 *
 * Description :
 *      Setup the magnetometer
 *      Registers are
 *      WIA2 - Device ID   READ ONLY Should return 9
 *      ST1  - Status 1    READ ONLY DOR (1) DRDY (0)
 *      XL   - Data low byte X
 *      XH
 *      YL
 *      YH
 *      ZL
 *      ZH 
 *      ST2   - Status 2  D3 - OVERVLOW (READ ONLY)
 *      CNTL2 - Control 2 (R/W)
 *          bit
 *           4 - Mode 4
 *           3 - Mode 3
 *           2 - Mode 2
 *           1 - Mode 1
 *           0 - Mode 0
 *      00000 - Power Down mode
 *      00001 - Single Measurement Mode
 *      00010 - Continious measurement mode 1
 *      00100 - Continious measurement mode 2
 *      00110 - Continious measurement mode 3
 *      01000 - Continious measurement mode 4
 *      10000 - Self test mode
 *
 *      CNTL3 - Control 3 , bit 0 SRST (R/W)
 * 
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::InitAK09916(uint8_t MagMode)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    ClearError(__LINE__);

    if (fMag_address<0)
    {
	log->Log("# Mag subsystem off.\n");
	return true;
    }
    // Initalize the mag sensor at the specified address.
    // the ioctl points us at the correct address. 
    if (ioctl(fdI2C, I2C_SLAVE, fMag_address) < 0)
    {
	SetError(-2, __LINE__);
 	log->LogTime(" Error opening AK09916, address 0x%2X\n", fMag_address);
	return false;
    }
    else
    {
	log->LogTime(" Opened AK09916, address 0x%2X\n", fMag_address);
    }

    fMagMode = MagMode;
    /*
     *      CNTL2 - Control 2 (R/W)
     *          bit
     *           4 - Mode 4
     *           3 - Mode 3
     *           2 - Mode 2
     *           1 - Mode 1
     *           0 - Mode 0
     *      00000 - Power Down mode
     *      00001 - Single Measurement Mode
     *      00010 - Continious measurement mode 1
     *      00100 - Continious measurement mode 2
     *      00110 - Continious measurement mode 3
     *      01000 - Continious measurement mode 4
     *      10000 - Self test mode
     *
     * 
     * CNTL2 0x31 Control (R/W) for magnetometer. 
     * 
     */
    WriteReg8(fMag_address, AK09916_CNTL2, fMagMode);

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

    fTemp = 0.0;
    if (fdI2C>=0)
    {
	uint8_t  Hi, Lo;
	uint16_t itemp;

	// Read out temperature sensor data Hi order byte
	//Hi = wiringPiI2CReadReg8(fIMU_address, TEMP_OUT_H);
	// Read out temperature sensor data Low order byte
	//Lo = wiringPiI2CReadReg8(fIMU_address, TEMP_OUT_L);

	// Turn the MSB and LSB into a 16-bit value
	Hi = ReadReg8( fIMU_address, TEMP_OUT_H);
	Lo = ReadReg8( fIMU_address,TEMP_OUT_L);
	itemp = ((int16_t)Hi << 8) | Lo;

	/*
	 * Temp C = ((TempOut - RoomTemp_Offset)/Temp_Sensitivity)+21.0
	 * I think RoomTemp_Offset is wrong. 
	 */
	fTemp = ((double)itemp - RoomTemp_Offset)/Temp_Sensitivity + 21.0;
#if 0
	cout << " Hi: " << (int) Hi
	     << " Lo: " << (int) Lo
	     << " itemp: " << itemp
	     << " ftemp: " << fTemp
	     << endl;
#endif
    }
    SET_DEBUG_STACK;
    return fTemp;
}

/**
 ******************************************************************
 *
 * Function Name : Read
 *
 * Description : Execute singleton read on all registers
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
bool ICM20948::Read(void)
{
    clock_gettime(CLOCK_REALTIME, &fReadTime);
    fReadTime.tv_sec -= fGMTOffset;
    readTempData();
    readAccelData();
    readGyroData();
    readMagData();
    return true;
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
bool ICM20948::readAccelData(void)
{
    SET_DEBUG_STACK;

    if (fIMU_address>0)
    {
	uint8_t *ptr;
	ptr = (uint8_t *)&itemp;

	// Read out sensor data Hi order byte
	*(ptr+1) = ReadReg8(fIMU_address, ACCEL_XOUT_H);
	// Read out sensor data Low order byte
	*ptr = ReadReg8(fIMU_address, ACCEL_XOUT_L);

	fAcc[0] = (double)itemp * fAres;

	// Read out sensor data Hi order byte
	*(ptr+1) = ReadReg8(fIMU_address, ACCEL_YOUT_H);
	// Read out sensor data Low order byte
	*ptr = ReadReg8(fIMU_address, ACCEL_YOUT_L);

	fAcc[1] = (double)itemp * fAres;

	// Read out sensor data Hi order byte
	*(ptr+1) = ReadReg8(fIMU_address, ACCEL_ZOUT_H);
	// Read out sensor data Low order byte
	*ptr = ReadReg8(fIMU_address, ACCEL_ZOUT_L);

	fAcc[2] = (double)itemp * fAres;

	SET_DEBUG_STACK;
	return true;
    }

    return false;
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
bool ICM20948::readGyroData(void)
{
    SET_DEBUG_STACK;

    if (fIMU_address>0)
    {
	uint8_t *ptr;
	ptr = (uint8_t *)&itemp;

	// Read out sensor data Hi order byte
	*(ptr+1) = ReadReg8(fIMU_address, GYRO_XOUT_H);
	// Read out sensor data Low order byte
	*ptr = ReadReg8(fIMU_address, GYRO_XOUT_L);

	fGyro[0] = (double)itemp * fGres;

	// Read out sensor data Hi order byte
	*(ptr+1) = ReadReg8(fIMU_address, GYRO_YOUT_H);
	// Read out sensor data Low order byte
	*ptr = ReadReg8(fIMU_address, GYRO_YOUT_L);

	fGyro[1] = (double)itemp * fGres;

	// Read out sensor data Hi order byte
	*(ptr+1) = ReadReg8(fIMU_address, GYRO_ZOUT_H);
	// Read out sensor data Low order byte
	*ptr = ReadReg8(fIMU_address, GYRO_ZOUT_L);

	fGyro[2] = (double)itemp * fGres;

	SET_DEBUG_STACK;
	return true;
    }

    return false;
}

/**
 ******************************************************************
 *
 * Function Name : readMagData
 *
 * Description :
 *     read the data from the magnetometer. 
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::readMagData(int16_t *results)
{
    SET_DEBUG_STACK;
    fMagRead = false;  // Assume read fails. 
    ClearError(__LINE__);
    uint8_t rv;

    if(fMag_address>0)
    {
	uint8_t *ptr;
	ptr = (uint8_t *)&itemp;

	/*
	 * must read ST2 at end of data acquisition.
	 *
	 * Wait for magnetometer data ready bit to be set
	 * Read Status register (ST1)
	 * Bit 0 - set is data ready
	 * Bit 1 set means data overrun. 
	 * DRDY bit turns to -Y�1� when data is ready in Single 
	 * measurement mode, Continuous measurement mode 1, 2, 3, 4 or 
	 * Self-test mode. It returns to �0� when any one of ST2 register 
	 * or measurement data register (HXL to TMPS) is read.
	 */
	if (fMagMode == 0x01)
	    rv = ReadReg8(fMag_address, AK09916_ST1);
	else
	    rv = 0x01;

	if (rv & 0x01)
	{
	    // 8 bytes of read. 
	    // Read the six raw data and ST2 registers sequentially into data array

	    // Read out sensor data Hi order byte
	    *(ptr+1) = ReadReg8(fMag_address, AK09916_XOUT_H);
	    // Read out sensor data Low order byte
	    *ptr = ReadReg8(fMag_address, AK09916_XOUT_L);

	    if(results) results[0] = itemp;
	    fMag[0] = (double)itemp * kMagRes;

	    // Read out sensor data Hi order byte
	    *(ptr+1) = ReadReg8(fMag_address, AK09916_YOUT_H);
	    // Read out sensor data Low order byte
	    *ptr = ReadReg8(fMag_address, AK09916_YOUT_L);

	    if(results) results[1] = itemp;
	    fMag[1] = (double)itemp * kMagRes;

	    // Read out sensor data Hi order byte
	    *(ptr+1) = ReadReg8(fMag_address, AK09916_ZOUT_H);
	    // Read out sensor data Low order byte
	    *ptr = ReadReg8(fMag_address, AK09916_ZOUT_L);

	    if(results) results[2] = itemp;
	    fMag[2] = (double)itemp * kMagRes;

	    /* 
	     * Data status  0x18, ST2
	     * End data read by reading ST2 register
	     * Section 13.4
	     * Bits
	     * 7     ZERO
	     * 6:4   - Reserved
	     * 3     0 Normal, 1 Magnetic sensor overflow. 
	     * 2:0   ZERO
	     * 
	     * Overflow is dependend on  mode. 
	     * Also signifies to magnetometer that the read is complete
	     * Section 13.4
	     * ST2 register has a role as data reading end register, also. 
	     * When any of measurement data register (HXL to TMPS) is read in 
	     * Continuous measurement mode 1, 2, 3, 4, it means data reading 
	     * start and taken as data reading until ST2 register is read. 
	     * Therefore, when any of measurement data is read, be sure to 
	     * read ST2 register at the end.
	     */
	    uint8_t st2 = ReadReg8(fMag_address, AK09916_ST2);
	    if (st2 & 0x08)
	    {
		cout << "OVERFLOW IN MAGNETOMETER." << endl;
	    }

	    //uint8_t int_status = ReadReg8(fMag_address, INT_STATUS);
	    fMagRead = true;
#if 0
	    // Something is not quite write with this code. 8
	    // bytes ends up at 0x19 rawdata[7]
	    // which is INT_STATUS

	    readBytes(AK09916_ADDRESS, AK09916_XOUT_L, 8, &rawData[0]);
	    uint8_t c = rawData[7]; // End data read by reading ST2 register
	    // Check if magnetic sensor overflow set, if not then report data
	    // Remove once finished
	
	    if (!(c & 0x08))
	    {
		// Turn the MSB and LSB into a signed 16-bit value
		destination[0] = ((int16_t)rawData[1] << 8) | rawData[0];
		// Data stored as little Endian
		destination[1] = ((int16_t)rawData[3] << 8) | rawData[2];
		destination[2] = ((int16_t)rawData[5] << 8) | rawData[4];
	    }
#endif
	}
    }

    SET_DEBUG_STACK;
    return fMagRead;
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

    CLogger *log = CLogger::GetThis();

    uint8_t selfTest[6];
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
    WriteReg8( fIMU_address,  PWR_MGMT_1, 0x01);
  
    // Switch to user bank 2
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    // Set gyro sample rate to 1 kHz
    WriteReg8( fIMU_address, GYRO_SMPLRT_DIV, 0x00);

    // Set gyro sample rate to 1 kHz, DLPF to 119.5 Hz and FSR to 250 dps
    WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x11);

    // Set accelerometer rate to 1 kHz and bandwidth to 111.4 Hz
    // Set full scale range for the accelerometer to 2 g
    WriteReg8( fIMU_address, ACCEL_CONFIG, 0x11);

    // Switch to user bank 0
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);

    // Hang out a bit, wait for the chip to stablize. 
    nanosleep(&sleeptime, NULL);

    // Get average current values of gyro and acclerometer
    for (int ii = 0; ii < kNAVG; ii++)
    {
	log->Log("# BHW::ii = %d\n", ii);

	// Read The acclerometer data and average it. 
	readAccelData();
	aAvg[0] += fAcc[0];
	aAvg[1] += fAcc[1];
	aAvg[2] += fAcc[2];

	// Read Gyro data. 
	readGyroData();
	gAvg[0] += fGyro[0];
	gAvg[1] += fGyro[1];
	gAvg[2] += fGyro[2];
    }

    // Get average of 200 values and store as average current readings
    for (int ii =0; ii < 3; ii++)
    {
	aAvg[ii] /= ((double)kNAVG);
	gAvg[ii] /= ((double)kNAVG);
    }
  
    // Switch to user bank 2
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    /*
     * Configure the accelerometer for self-test
     * Enable self test on all three axes and set accelerometer 
     * range to +/- 2 g
     */
    WriteReg8( fIMU_address, ACCEL_CONFIG_2, 0x1C);

    /*
     * Enable self test on all three axes and set gyro range 
     * to +/- 250 degrees/s
     */
    WriteReg8( fIMU_address, GYRO_CONFIG_2,  0x38);

  
    // Switch to user bank 0
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);

    // Hang out a bit, wait for the chip to stablize. 
    nanosleep(&sleeptime, NULL);

    // Get average self-test values of gyro and acclerometer
    for (int ii = 0; ii < kNAVG; ii++)
    {
	readAccelData();
	aSTAvg[0] += fAcc[0];
	aSTAvg[1] += fAcc[1];
	aSTAvg[2] += fAcc[2];

	// Read Gyro data. 
	readGyroData();
	gSTAvg[0] += fGyro[0];
	gSTAvg[1] += fGyro[1];
	gSTAvg[2] += fGyro[2];
    }

    // Get average of 200 values and store as average self-test readings
    for (int ii =0; ii < 3; ii++)
    {
	aSTAvg[ii] /= ((double)kNAVG);
	gSTAvg[ii] /= ((double)kNAVG);
    }
  
    // Switch to user bank 2
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    // Configure the gyro and accelerometer for normal operation
    WriteReg8( fIMU_address, ACCEL_CONFIG_2, 0x00);
    WriteReg8( fIMU_address, GYRO_CONFIG_2,  0x00);
    
  
    // Switch to user bank 1
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x10);

    /*
     * Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
     * X-axis accel self-test results
     */
    selfTest[0] = ReadReg8( fIMU_address, SELF_TEST_X_ACCEL);
    // Y-axis accel self-test results
    selfTest[1] = ReadReg8( fIMU_address, SELF_TEST_Y_ACCEL);
    // Z-axis accel self-test results
    selfTest[2] = ReadReg8( fIMU_address, SELF_TEST_Z_ACCEL);
    // X-axis gyro self-test results
    selfTest[3] = ReadReg8( fIMU_address, SELF_TEST_X_GYRO);
    // Y-axis gyro self-test results
    selfTest[4] = ReadReg8( fIMU_address, SELF_TEST_Y_GYRO);
    // Z-axis gyro self-test results
    selfTest[5] = ReadReg8( fIMU_address, SELF_TEST_Z_GYRO);
  
    // Switch to user bank 0
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);
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
/**
 ******************************************************************
 *
 * Function Name : magCalICM20948
 *
 * Description :
 *     Function which accumulates magnetometer data after device 
 *     initialization.
 *     It calculates the bias and scale in the x, y, and z axes.
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void ICM20948::magCalICM20948(double * bias_dest, double * scale_dest)
{
    SET_DEBUG_STACK;

    const struct timespec D135ms = {0L, 135000000L}; // 135ms
    const struct timespec D10ms  = {0L,  10000000L}; //  10ms

    CLogger *log = CLogger::GetThis();

    uint16_t ii = 0, sample_count = 0;
    int32_t mag_bias[3]  = {0, 0, 0};
    int32_t mag_scale[3] = {0, 0, 0};
    int16_t mag_max[3]   = { SHRT_MIN, SHRT_MIN, SHRT_MIN};
    int16_t mag_min[3]   = { SHRT_MAX, SHRT_MAX, SHRT_MAX};
    int16_t mag_temp[3]  = {0, 0, 0};

    // Make sure resolution has been calculated
    getMres();

    cout << "Mag Calibration: Wave device in a figure 8 until done!" << endl
	 << "  4 seconds to get ready followed by 15 seconds of sampling)" 
	 << endl;
    sleep(4);

    // shoot for ~fifteen seconds of mag data
    // at 8 Hz ODR, new mag data is available every 125 ms
    switch(fMmode)
    {
    case M_8HZ:
	sample_count = 128;
	break;
    case M_100HZ: // at 100 Hz ODR, new mag data is available every 10 ms
	sample_count = 1500;
	break;
    }

    /*
     * Loop and obtain data for 15 seconds. 
     * Find the max and min of each. 
     */
    for (ii = 0; ii < sample_count; ii++)
    {
        // Read the mag data, store the integer data in mag temp.
	readMagData(mag_temp);  

	/*
	 * Loop over all three axis. 
	 */
	for (int jj = 0; jj < 3; jj++)
	{
	    if (mag_temp[jj] > mag_max[jj])
	    {
		mag_max[jj] = mag_temp[jj];
	    }
	    if (mag_temp[jj] < mag_min[jj])
	    {
		mag_min[jj] = mag_temp[jj];
	    }
	}

	switch(fMmode) 
	{
	case M_8HZ:
	    nanosleep(&D135ms, NULL);
	    break;
	case M_100HZ:
            // At 100 Hz ODR, new mag data is available every 10 ms
	    nanosleep(&D10ms, NULL);
	    break;
	}
    }

    log->Log("# Mag X Max: %f, Min: %f \n", mag_max[0], mag_min[0]);
    log->Log("# Mag Y Max: %f, Min: %f \n", mag_max[1], mag_min[1]);
    log->Log("# Mag Z Max: %f, Min: %f \n", mag_max[2], mag_min[2]);

    /*
     * Get hard iron correction
     * Get 'average' x mag bias in counts
     */
    mag_bias[0]  = (mag_max[0] + mag_min[0]) / 2;
    // Get 'average' y mag bias in counts
    mag_bias[1]  = (mag_max[1] + mag_min[1]) / 2;
    // Get 'average' z mag bias in counts
    mag_bias[2]  = (mag_max[2] + mag_min[2]) / 2;

    // Save mag biases in G for main program
    bias_dest[0] = (double)mag_bias[0] * kMagRes;// * factoryMagCalibration[0];
    bias_dest[1] = (double)mag_bias[1] * kMagRes;// * factoryMagCalibration[1];
    bias_dest[2] = (double)mag_bias[2] * kMagRes;// * factoryMagCalibration[2];

    /*
     * Get soft iron correction estimate
     * Get average x axis max chord length in counts
     */
    mag_scale[0]  = (mag_max[0] - mag_min[0]) / 2;
    // Get average y axis max chord length in counts
    mag_scale[1]  = (mag_max[1] - mag_min[1]) / 2;
    // Get average z axis max chord length in counts
    mag_scale[2]  = (mag_max[2] - mag_min[2]) / 2;

    double avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
    avg_rad /= 3.0;

    scale_dest[0] = avg_rad / ((double)mag_scale[0]);
    scale_dest[1] = avg_rad / ((double)mag_scale[1]);
    scale_dest[2] = avg_rad / ((double)mag_scale[2]);

    log->LogTime(" Mag Calibration done!");
    log->Log("# Bias X: %f, Y: %f, Z: %f\n", 
	     bias_dest[0], bias_dest[1], bias_dest[2]);
    log->Log("# scale X: %f, Y: %f, Z: %f\n", 
	     mag_scale[0], mag_scale[1], mag_scale[2]);
    SET_DEBUG_STACK;
}



/**
 ******************************************************************
 *
 * Function Name :  <<
 *
 * Description : operator to print out values from last read
 *      switches output based on what sensors are operating. 
 *
 * Inputs : 
 *     output - ostream to place data into
 *     n - ICM20948 class. 
 *
 * Returns : populated ostream
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
ostream& operator<<(ostream& output, const ICM20948 &n)
{
    SET_DEBUG_STACK;
    //output << (CObject)n << (IMUData)n;
    output << (IMUData)n;
    return output;
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
    WriteReg8( fIMU_address, PWR_MGMT_1, READ_FLAG);
    delay(200);

    // get stable time source; Auto select clock source to be PLL gyroscope
    // reference if ready else use the internal oscillator, bits 2:0 = 001
    WriteReg8( fIMU_address, PWR_MGMT_1, 0x01);
    delay(200);

    // Configure device for bias calculation
    // Disable all interrupts
    WriteReg8( fIMU_address, INT_ENABLE, 0x00);
    // Disable FIFO
    WriteReg8( fIMU_address, FIFO_EN_1, 0x00);
    WriteReg8( fIMU_address, FIFO_EN_2, 0x00);
    // Turn on internal clock source
    WriteReg8( fIMU_address, PWR_MGMT_1, 0x00);
    // Disable I2C master
    //WriteReg8( fIMU_address, I2C_MST_CTRL, 0x00); Already disabled
    // Disable FIFO and I2C master modes
    WriteReg8( fIMU_address, USER_CTRL, 0x00);
    // Reset FIFO and DMP
    WriteReg8( fIMU_address, USER_CTRL, 0x08);
    WriteReg8( fIMU_address, FIFO_RST, 0x1F);
    delay(10);
    WriteReg8( fIMU_address, FIFO_RST, 0x00);
    delay(15);

    // Set FIFO mode to snapshot
    WriteReg8( fIMU_address, FIFO_MODE, 0x1F);
    // Switch to user bank 2
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);
    // Configure ICM20948 gyro and accelerometer for bias calculation
    // Set low-pass filter to 188 Hz
    WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x01);
    // Set sample rate to 1 kHz
    WriteReg8( fIMU_address, GYRO_SMPLRT_DIV, 0x00);
    // Set gyro full-scale to 250 degrees per second, maximum sensitivity
    WriteReg8( fIMU_address, GYRO_CONFIG_1, 0x00);
    // Set accelerometer full-scale to 2 g, maximum sensitivity
    WriteReg8( fIMU_address, ACCEL_CONFIG, 0x00);

    uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
    uint16_t  accelsensitivity = 16384; // = 16384 LSB/g

    // Switch to user bank 0
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);
    // Configure FIFO to capture accelerometer and gyro data for bias calculation
    WriteReg8( fIMU_address, USER_CTRL, 0x40);  // Enable FIFO
    // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in
    // ICM20948)
    WriteReg8( fIMU_address, FIFO_EN_2, 0x1E);
    delay(40);  // accumulate 40 samples in 40 milliseconds = 480 bytes

    // At end of sample accumulation, turn off FIFO sensor read
    // Disable gyro and accelerometer sensors for FIFO
    WriteReg8( fIMU_address, FIFO_EN_2, 0x00);
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
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x20);

    // Push gyro biases to hardware registers
    WriteReg8( fIMU_address, XG_OFFSET_H, data[0]);
    WriteReg8( fIMU_address, XG_OFFSET_L, data[1]);
    WriteReg8( fIMU_address, YG_OFFSET_H, data[2]);
    WriteReg8( fIMU_address, YG_OFFSET_L, data[3]);
    WriteReg8( fIMU_address, ZG_OFFSET_H, data[4]);
    WriteReg8( fIMU_address, ZG_OFFSET_L, data[5]);

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
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x10);
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
    WriteReg8( fIMU_address, XA_OFFSET_H, data[0]);
    WriteReg8( fIMU_address, XA_OFFSET_L, data[1]);
    WriteReg8( fIMU_address, YA_OFFSET_H, data[2]);
    WriteReg8( fIMU_address, YA_OFFSET_L, data[3]);
    WriteReg8( fIMU_address, ZA_OFFSET_H, data[4]);
    WriteReg8( fIMU_address, ZA_OFFSET_L, data[5]);

    // Output scaled accelerometer biases for display in the main program
    accelBias[0] = (float)accel_bias[0]/(float)accelsensitivity;
    accelBias[1] = (float)accel_bias[1]/(float)accelsensitivity;
    accelBias[2] = (float)accel_bias[2]/(float)accelsensitivity;
    // Switch to user bank 0
    WriteReg8( fIMU_address, REG_BANK_SEL, 0x00);
}

#endif
/**
 ******************************************************************
 *
 * Function Name : ReadReg8
 *
 * Description :
 *     Read a single byte of data from the specified register. eg. 
 *     8 bit wide register.
 *
 * Inputs : Register to read from 
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
uint8_t ICM20948::ReadReg8(uint8_t SlaveAddress, uint8_t Register)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    ClearError(__LINE__);
    uint8_t rv = 0;
    struct i2c_smbus_ioctl_data blk;
    union  i2c_smbus_data       i2cdata;

    blk.read_write = 1;
    blk.command    = Register;
    blk.size       = I2C_SMBUS_BYTE_DATA;
    blk.data       = &i2cdata;

    if (fdI2C>=0)
    {
	/*
	 * Sequence
	 * 1) Select slave I2C address with write bit 0
	 * 2) Send internal register address. 
	 * 3) Select slave I2C address with read bit 1
	 * 4) Read the data.  no ack
	 */
	
	// We are pointing at the "slave" on the I2C bus. 
	if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
	{
	    SetError(-2, __LINE__);
	    log->LogTime(" Error selecting ICM20948 for read, address 0x%2X\n",
			 SlaveAddress);
	    return false;
	}

	if(ioctl(fdI2C, I2C_SMBUS, &blk) < 0)
	{
	    SetError(-3, __LINE__);
	    return rv;
	}
	rv = (uint8_t) i2cdata.byte;
    }

    SET_DEBUG_STACK;
    return rv;
}
/**
 ******************************************************************
 *
 * Function Name : ReadBlock
 *
 * Description : Read N byte from the I2C (maximum of 31 bytes possible)
 *     
 *
 * Inputs : 
 *     SlaveAddress - address of the subsystem on the bus.
 *     Register - to read from
 *     size     - number of bytes to read
 *     data     - user provided data array to store data. 
 *
 * Returns : 
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
int ICM20948::ReadBlock(uint8_t SlaveAddress, unsigned char Register, 
			size_t  size,  void* data)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    ClearError(__LINE__);
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	SetError(-2, __LINE__);
	log->LogTime(" Error selecting ICM20948 for read, address 0x%2X\n",
		     SlaveAddress);
	return -1;
    }

    blk.read_write   = 1;
    blk.command      = Register;
    blk.size         = I2C_SMBUS_I2C_BLOCK_DATA;
    blk.data         = &i2cdata;
    i2cdata.block[0] = size;

    if(ioctl(fdI2C, I2C_SMBUS, &blk) < 0)
    {
	SetError(-4, __LINE__);
	return -1;
    }

    memcpy( data, &i2cdata.block[1], size);
    SET_DEBUG_STACK;
    return   i2cdata.block[0];
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description :
 *     
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */


/**
 ******************************************************************
 *
 * Function Name : WriteReg8
 *
 * Description :
 *     SlaveAddress - address of the subsystem on the bus.
 *     Register - to read from
 *     value    - value to write to 8 bit wide register
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool ICM20948::WriteReg8(uint8_t SlaveAddress, 
			 unsigned char Register, unsigned char value)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    ClearError(__LINE__);
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	SetError(-2, __LINE__);
	log->LogTime(" Error selecting ICM20948 for read, address 0x%2X\n",
		     SlaveAddress);
	return -1;
    }


    i2cdata.byte   = value;
    blk.read_write = 0;
    blk.command    = Register;
    blk.size       = I2C_SMBUS_BYTE_DATA;
    blk.data       = &i2cdata;

    if(ioctl(fdI2C, I2C_SMBUS, &blk)<0)
    {
	log->LogTime(" Unable to write I2C byte data\n");
	SET_DEBUG_STACK;
	SetError(-4, __LINE__);
        return false;
    }
    SET_DEBUG_STACK;
    return true;
}


/**
 ******************************************************************
 *
 * Function Name : ReadWord
 *
 * Description : Read a 16 bit word on the I2C bus. 
 *     SlaveAddress - address of the subsystem on the bus.
 *     Register - to read from
 *     
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
int ICM20948::ReadWord(uint8_t SlaveAddress, unsigned char Register)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    ClearError(__LINE__);
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	SetError(-2, __LINE__);
	log->LogTime(" Error selecting ICM20948 for read, address 0x%2X\n",
		     SlaveAddress);
	return -1;
    }

    blk.read_write = 1;
    blk.command    = Register;
    blk.size       = I2C_SMBUS_WORD_DATA;
    blk.data       = &i2cdata;

    if(ioctl( fdI2C, I2C_SMBUS, &blk)<0)
    {
	SetError(-5, __LINE__);
	log->LogTime(" Error ICM20948 for readword\n");
	return -1;
    }
    return  (int) i2cdata.word;
}


/**
 ******************************************************************
 *
 * Function Name : WriteWord
 *
 * Description :
 *     SlaveAddress - address of the subsystem on the bus.
 *     Register - to read from
 *     value    - value to write to 8 bit wide register
 *     
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
////////////////////////////////////   I2CWrapperWriteWord
//
//    Write 2 bytes  to the I2C device
//
//     inputs,
//
//     handle:   IO handle
//     cmd:  Specify which is the device command (more or less the device function or register)
//     value:    byte value
//    Return   number of byte written if <0 error
//
//    Check I2CWrapperErrorFlag for error
//
bool ICM20948::WriteWord(uint8_t SlaveAddress, uint8_t Register, 
			 uint16_t value)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    ClearError(__LINE__);
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	SetError(-2, __LINE__);
	log->LogTime(" Error selecting ICM20948, WriteWord, address 0x%2X\n",
		     SlaveAddress);
	return false;
    }

    i2cdata.word   = value;
    blk.read_write = 0;
    blk.command    = Register;
    blk.size       = I2C_SMBUS_WORD_DATA;
    blk.data       = &i2cdata;

    if(ioctl( fdI2C, I2C_SMBUS, &blk)<0)
    {
	SetError(-6, __LINE__);
	log->LogTime(" Error ICM20948 for WriteWord\n");
	return false;
    }
    return true;
}
