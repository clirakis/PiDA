/**
 ******************************************************************
 *
 * Module Name : I2CHelper.h
 *
 * Author/Date : C.B. Lirakis / 29-Mar-24
 *
 * Description : Moved all I2C communications wrappers here. 
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 * http://wiringpi.com/reference/i2c-library/
 * https://github.com/drcpattison/ICM-20948/  - USED A LOT OF CODE FROM THIS
 * https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface
 * This is particularly helpful:
 * https://github.com/danjperron/A2D_PIC_RPI/blob/master/I2CWrapper.c
 *
 *******************************************************************
 */
#ifndef __I2CHELPER_hh_
#define __I2CHELPER_hh_
#    include <stdint.h>
#    include <fcntl.h>
#    include <sys/ioctl.h>
// specific to talking to I2C
#    include <linux/i2c-dev.h>
#    include <linux/i2c.h>

/// I2CHelper documentation here. 
class I2CHelper {
public:
    /// Default Constructor
    I2CHelper(const char *I2CDeviceName);
    /// Default destructor
    ~I2CHelper();
    /// I2CHelper function

    inline bool Error(void) {return fError;};
    inline int  FD(void)    {return fdI2C;};

    uint8_t ReadReg8(uint8_t SlaveAddress, uint8_t Register);
    int     ReadBlock(uint8_t SlaveAddress, unsigned char Register, 
		  size_t  size,  void* data);

    bool    WriteReg8(uint8_t SlaveAddress, unsigned char Register, 
		   unsigned char value);

    int     ReadWord(uint8_t SlaveAddress, unsigned char Register);

    bool    WriteWord(uint8_t SlaveAddress, uint8_t Register, 
			  uint16_t value);
    /*! Access the This pointer. */
    static I2CHelper* GetThis(void) {return fI2C;};

    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */

private:
    int    fdI2C;   // Pointer to I2C device. 
    bool   fError;  // returns true if error on last function call. 

    /*! The static 'this' pointer. */
    static I2CHelper *fI2C;

};
#endif
