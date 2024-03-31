/********************************************************************
 *
 * Module Name : I2CHelper.cpp
 *
 * Author/Date : C.B. Lirakis / 29-Mar-24
 *
 * Description : Generic I2CHelper
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
#include <cstring>


// Local Includes.
#include "debug.h"
#include "CLogger.hh"
#include "I2CHelper.hh"

I2CHelper* I2CHelper::fI2C;

/**
 ******************************************************************
 *
 * Function Name : I2CHelper constructor
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
I2CHelper::I2CHelper (const char *DeviceName)
{
    SET_DEBUG_STACK;
    fError = false;
    fI2C   = this;
    fdI2C  = open( DeviceName, O_RDWR);
    fError = (fdI2C<0);
}

/**
 ******************************************************************
 *
 * Function Name : I2CHelper destructor
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
I2CHelper::~I2CHelper (void)
{
    SET_DEBUG_STACK;
    close(fdI2C);
}


/**
 ******************************************************************
 *
 * Function Name : I2CHelper function
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
uint8_t I2CHelper::ReadReg8(uint8_t SlaveAddress, uint8_t Register)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    uint8_t rv = 0;
    struct i2c_smbus_ioctl_data blk;
    union  i2c_smbus_data       i2cdata;

    fError = false;

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
	    
	    log->LogTime(" Error selecting I2CHelper for read, address 0x%2X\n",
			 SlaveAddress);
	    fError = true;
	    return -1;
	}

	if(ioctl(fdI2C, I2C_SMBUS, &blk) < 0)
	{
	    fError = true;
	    return -1;
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
int I2CHelper::ReadBlock(uint8_t SlaveAddress, unsigned char Register, 
			size_t  size,  void* data)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    fError = false;
    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	fError = true;
	log->LogTime(" Error selecting I2CHelper for read, address 0x%2X\n",
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
	fError = true;
	return -1;
    }

    memcpy( data, &i2cdata.block[1], size);
    SET_DEBUG_STACK;
    return   i2cdata.block[0];
}

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
bool I2CHelper::WriteReg8(uint8_t SlaveAddress, 
			 unsigned char Register, unsigned char value)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    fError = false;

    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	fError = true;
	log->LogTime(" Error selecting I2CHelper for read, address 0x%2X\n",
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
	fError = true;
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
int I2CHelper::ReadWord(uint8_t SlaveAddress, unsigned char Register)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    fError = false;
    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	fError = true;
	log->LogTime(" Error selecting I2CHelper for read, address 0x%2X\n",
		     SlaveAddress);
	return -1;
    }

    blk.read_write = 1;
    blk.command    = Register;
    blk.size       = I2C_SMBUS_WORD_DATA;
    blk.data       = &i2cdata;

    if(ioctl( fdI2C, I2C_SMBUS, &blk)<0)
    {
	fError = true;
	log->LogTime(" Error I2CHelper for readword\n");
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
bool I2CHelper::WriteWord(uint8_t SlaveAddress, uint8_t Register, 
			 uint16_t value)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();
    struct i2c_smbus_ioctl_data  blk;
    union i2c_smbus_data         i2cdata;

    fError = false;

    // We are pointing at the "slave" on the I2C bus. 
    if (ioctl(fdI2C, I2C_SLAVE, SlaveAddress) < 0)
    {
	fError = true;
	log->LogTime(" Error selecting I2CHelper, WriteWord, address 0x%2X\n",
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
	fError = true;
	log->LogTime(" Error I2CHelper for WriteWord\n");
	return false;
    }
    return true;
}
