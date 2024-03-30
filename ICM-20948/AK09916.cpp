/********************************************************************
 *
 * Module Name : AK09916.cpp
 *
 * Author/Date : C.B. Lirakis / 29-Mar-24
 *
 * Description : Generic AK09916
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <unistd.h>
#include <climits>
#include <cstring>

// Local Includes.
#include "AK09916.hh"
#include "I2CHelper.hh"
#include "debug.h"
#include "CLogger.hh"

/**
 ******************************************************************
 *
 * Function Name : AK09916 constructor
 *
 * Description :
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
 * Status byte returned after each command, composed of (section 15.2)
 *    BIT
 *     7   BURST MODE
 *     6   WOC_MODE
 *     5   SM_MODE
 *     4   ERROR - command rejected
 *     3   SED   - single error detection
 *     2   RS    - acknowledge reset
 *     1   D1    - Depends on query
 *     0   D0
 *     
 * 15.3.1.1
 *     Commands SB, SWOC, SM, EX, HR, HS
 *          example  0x16 0x00
 *                   xx   Status
 *
 * HS command needs a 15ms wait before next command is issued. 
 * 
 * RT - Warm reset 0xF0 < Status> 1 byte 
 *
 * RM - read magnetic. 0x4F 7 bytes in 
 *                     0x45 6 bytes in
 *
 * RR - read register 0x50 Register<<2 0x00 0x00 (5 bytes)
 *
 * WR - write register. 
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
AK09916::AK09916 (uint8_t Address, uint8_t Mode)
{
    SET_DEBUG_STACK;
    CLogger   *pLog = CLogger::GetThis();
    I2CHelper *pI2C = I2CHelper::GetThis();

    fError      = false;
    fMagMode    = Mode;
    fMmode      = M_100HZ;   // probably want to put external FIXME
    fMagAddress = Address; 

    if (fMagAddress<0)
    {
	pLog->Log("# Mag subsystem off.\n");
	fError = false;;
    }
    // Initalize the mag sensor at the specified address.
    // the ioctl points us at the correct address. 
    if (ioctl(pI2C->FD(), I2C_SLAVE, fMagAddress) < 0)
    {
 	pLog->LogTime(" Error opening AK09916, address 0x%2X\n", fMagAddress);
	fError = true;
    }
    else
    {
	pLog->LogTime(" Opened AK09916, address 0x%2X\n", fMagAddress);
    }

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
    pI2C->WriteReg8(fMagAddress, AK09916_CNTL2, fMagMode);

    SET_DEBUG_STACK;
    fError = false;
}

/**
 ******************************************************************
 *
 * Function Name : AK09916 destructor
 *
 * Description :
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
AK09916::~AK09916 (void)
{
}


/**
 ******************************************************************
 *
 * Function Name : AK09916 function
 *
 * Description :
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
bool AK09916::Read(int16_t *results)
{
    SET_DEBUG_STACK;
    CLogger   *pLog = CLogger::GetThis();
    I2CHelper *pI2C = I2CHelper::GetThis();
    uint8_t   rv;
    uint16_t  itemp;
    
    fError   = false;
    fMagRead = false; // Read failed. 

    if(fMagAddress>0)
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
	 * DRDY bit turns to -Y´1¡ when data is ready in Single 
	 * measurement mode, Continuous measurement mode 1, 2, 3, 4 or 
	 * Self-test mode. It returns to ´0¡ when any one of ST2 register 
	 * or measurement data register (HXL to TMPS) is read.
	 */
	if (fMagMode == 0x01)
	    rv = pI2C->ReadReg8(fMagAddress, AK09916_ST1);
	else
	    rv = 0x01;

	if (rv & 0x01)
	{
	    // 8 bytes of read. 
	    // Read the six raw data and ST2 registers sequentially into data array

	    // Read out sensor data Hi order byte
	    *(ptr+1) = pI2C->ReadReg8(fMagAddress, AK09916_XOUT_H);
	    // Read out sensor data Low order byte
	    *ptr = pI2C->ReadReg8(fMagAddress, AK09916_XOUT_L);

	    if(results) results[0] = itemp;
	    fMag[0] = (double)itemp * kMagRes;

	    // Read out sensor data Hi order byte
	    *(ptr+1) = pI2C->ReadReg8(fMagAddress, AK09916_YOUT_H);
	    // Read out sensor data Low order byte
	    *ptr = pI2C->ReadReg8(fMagAddress, AK09916_YOUT_L);

	    if(results) results[1] = itemp;
	    fMag[1] = (double)itemp * kMagRes;

	    // Read out sensor data Hi order byte
	    *(ptr+1) = pI2C->ReadReg8(fMagAddress, AK09916_ZOUT_H);
	    // Read out sensor data Low order byte
	    *ptr = pI2C->ReadReg8(fMagAddress, AK09916_ZOUT_L);

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
	    uint8_t st2 = pI2C->ReadReg8(fMagAddress, AK09916_ST2);
	    if (st2 & 0x08)
	    {
		pLog->LogTime("OVERFLOW IN MAGNETOMETER.\n");
		fError = true;
	    }

	    //uint8_t int_status = ReadReg8(fMagAddress, INT_STATUS);
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
void AK09916::Copy(double *array)
{
    SET_DEBUG_STACK;
    if (array)
	memcpy(array, fMag, 3*sizeof(double));
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
void AK09916::Calibrate(double * bias_dest, double * scale_dest)
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
	Read(mag_temp);  

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

