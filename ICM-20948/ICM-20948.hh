/**
 ******************************************************************
 *
 * Module Name : ICM-20948.h
 *
 * Author/Date : C.B. Lirakis / 25-Feb-22
 *
 * Description : Interface to Intel ICM-20948 9DOF I2C sensor
 * This has a sub chip in it for the magnetic sensing AK09916, 
 *
 *  FIFO 512 deep
 *  Gyroscope +/- 250 dps, +/- 500 dps, +/- 1000 dps, +/- 2000 dps
 *  Accelerometer +/- 2g, +/- 4g, +/- 8g, +/- 16g
 *  16 bit ADC
 *  Temperature sensor
 *  Magnetometer is +/- 4900uT
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 * https://www.sparkfun.com/products/15335
 * https://learn.sparkfun.com/tutorials/raspberry-pi-spi-and-i2c-tutorial#i2c-on-pi
 * https://github.com/drcpattison/ICM-20948/blob/master/src/ICM20948.h
 *
 *******************************************************************
 */
#ifndef __ICM20948_hh_
#define __ICM20948_hh_
#   include <inttypes.h>
#   include <time.h>
#   include "CObject.hh"
#   include "IMUData.hh"    // Keep all the IMU data in one class

/// Define Registers here
// See also ICM-20948 Datasheet, Register Map and Descriptions, Revision 1.3,
// https://www.invensense.com/wp-content/uploads/2016/06/DS-000189-ICM-20948-v1.3.pdf

//Magnetometer Registers - NOTE THIS is at a different I2C address    
#define AK09916_ADDRESS         0x0C 
#define WHO_AM_I_AK09916        0x01 // (AKA WIA2) should return 0x09
#define AK09916_ST1             0x10  // data ready status bit 0
#define AK09916_XOUT_L          0x11  // data
#define AK09916_XOUT_H          0x12
#define AK09916_YOUT_L          0x13
#define AK09916_YOUT_H          0x14
#define AK09916_ZOUT_L          0x15
#define AK09916_ZOUT_H          0x16
// Data overflow bit 3 and data read error status bit 2
#define AK09916_ST2             0x18  

// Power down (0000), single-measurement (0001), self-test (1000) 
// and Fuse ROM (1111) modes on bits 3:0
#define AK09916_CNTL2           0x31  // Normal (0), Reset (1)

// ICM-20948 ---------------------------------------------------------

// USER BANK 0 REGISTER MAP
#define WHO_AM_I_ICM20948       0x00 // Should return 0xEA
#define USER_CTRL               0x03
#define LP_CONFIG	        0x05
#define PWR_MGMT_1              0x06 // Device defaults to the SLEEP mode
#define PWR_MGMT_2              0x07
#define INT_PIN_CFG             0x0F
#define INT_ENABLE              0x10
#define INT_ENABLE_1	        0x11
#define INT_ENABLE_2	        0x12
#define INT_ENABLE_3	        0x13
#define I2C_MST_STATUS          0x17
#define INT_STATUS              0x19
#define INT_STATUS_1	        0x1A
#define INT_STATUS_2	        0x1B
#define INT_STATUS_3	        0x1C
#define DELAY_TIMEH	        0x28
#define DELAY_TIMEL	        0x29
#define ACCEL_XOUT_H            0x2D
#define ACCEL_XOUT_L            0x2E
#define ACCEL_YOUT_H            0x2F
#define ACCEL_YOUT_L            0x30
#define ACCEL_ZOUT_H            0x31
#define ACCEL_ZOUT_L            0x32
#define GYRO_XOUT_H             0x33
#define GYRO_XOUT_L             0x34
#define GYRO_YOUT_H             0x35
#define GYRO_YOUT_L             0x36
#define GYRO_ZOUT_H             0x37
#define GYRO_ZOUT_L             0x38
#define TEMP_OUT_H              0x39
#define TEMP_OUT_L              0x3A
#define EXT_SENS_DATA_00        0x3B
#define EXT_SENS_DATA_01        0x3C
#define EXT_SENS_DATA_02        0x3D
#define EXT_SENS_DATA_03        0x3E
#define EXT_SENS_DATA_04        0x3F
#define EXT_SENS_DATA_05        0x40
#define EXT_SENS_DATA_06        0x41
#define EXT_SENS_DATA_07        0x42
#define EXT_SENS_DATA_08        0x43
#define EXT_SENS_DATA_09        0x44
#define EXT_SENS_DATA_10        0x45
#define EXT_SENS_DATA_11        0x46
#define EXT_SENS_DATA_12        0x47
#define EXT_SENS_DATA_13        0x48
#define EXT_SENS_DATA_14        0x49
#define EXT_SENS_DATA_15        0x4A
#define EXT_SENS_DATA_16        0x4B
#define EXT_SENS_DATA_17        0x4C
#define EXT_SENS_DATA_18        0x4D
#define EXT_SENS_DATA_19        0x4E
#define EXT_SENS_DATA_20        0x4F
#define EXT_SENS_DATA_21        0x50
#define EXT_SENS_DATA_22        0x51
#define EXT_SENS_DATA_23        0x52
#define FIFO_EN_1               0x66
#define FIFO_EN_2               0x67
#define FIFO_RST	        0x68
#define FIFO_MODE	        0x69
#define FIFO_COUNTH             0x70
#define FIFO_COUNTL             0x71
#define FIFO_R_W                0x72
#define DATA_RDY_STATUS	        0x74
#define FIFO_CFG	        0x76
#define REG_BANK_SEL	        0x7F

// USER BANK 1 REGISTER MAP
#define SELF_TEST_X_GYRO        0x02
#define SELF_TEST_Y_GYRO        0x03
#define SELF_TEST_Z_GYRO        0x04
#define SELF_TEST_X_ACCEL       0x0E
#define SELF_TEST_Y_ACCEL       0x0F
#define SELF_TEST_Z_ACCEL 	0x10
#define XA_OFFSET_H       	0x14
#define XA_OFFSET_L       	0x15
#define YA_OFFSET_H       	0x17
#define YA_OFFSET_L       	0x18
#define ZA_OFFSET_H       	0x1A
#define ZA_OFFSET_L       	0x1B
#define TIMEBASE_CORRECTION_PLL	0x28

// USER BANK 2 REGISTER MAP
#define GYRO_SMPLRT_DIV        	0x00
#define GYRO_CONFIG_1      	0x01
#define GYRO_CONFIG_2          	0x02
#define XG_OFFSET_H            	0x03  // User-defined trim values for gyroscope
#define XG_OFFSET_L            	0x04
#define YG_OFFSET_H            	0x05
#define YG_OFFSET_L            	0x06
#define ZG_OFFSET_H            	0x07
#define ZG_OFFSET_L            	0x08
#define ODR_ALIGN_EN	       	0x09 
#define ACCEL_SMPLRT_DIV_1     	0x10 
#define ACCEL_SMPLRT_DIV_2     	0x11 
#define ACCEL_INTEL_CTRL       	0x12 
#define ACCEL_WOM_THR	       	0x13 
#define ACCEL_CONFIG           	0x14
#define ACCEL_CONFIG_2         	0x15 
#define FSYNC_CONFIG	       	0x52 
#define TEMP_CONFIG	      	0x53 
#define MOD_CTRL_USR	       	0x54 

/// ICM-20948 documentation here. 
class ICM20948 : public CObject, public IMUData
{
public:

    /*!
     * Description:  Constructor for ICM20948, also initializes AK09916
     *               magnetometer chip. 
     *
     * Arguments:
     *     IMU_address - I2C address where IICM20948 resides (0x69)
     *     Mag_address - I2C address where AK09916 resides  (0x0C)
     *
     * Returns:
     *     fully constructed class
     *
     * Errors:
     *     If the open and configure of either sensor fails. 
     *
     */
    ICM20948(uint8_t IMU_address, uint8_t Mag_address);


    /*!
     * Description: 
     *   Destructor for IMU setup
     *   Not much done at this time. 
     *
     * Arguments:
     *   NONE
     *
     * Returns:
     *   NONE
     * 
     * Errors:
     *
     */
    ~ICM20948(void);

    /* Get data from all sensors. */
    bool Read(void);

    /*!
     * Magnetic sensor resolution
     * Sections: 2.3, 
     * Earth magnetic field ~(25-65 uT)
     * 4912.0 uT max scale, can go +/-
     * 16 bits bipolar.
     * The code I stole this from had 10.0 multiplier, I don't know why. 
     * Check cal. 
     */
    const double kMagRes = (4912.0/32760.0);

    /*!
     * Description: 
     *   Get the magnetic resolution
     *
     * Arguments:
     *   None
     *
     * Returns:
     *   Magnetic resolution in uT
     * 
     * Errors:
     *
     */
    inline double getMres(void) {return kMagRes;};

    /*!
     * Description: 
     *   returns the resloution of the gryo in dps based on the
     *   configuration. 
     *
     * Arguments:
     *   NONE
     *
     * Returns:
     *   Gryo dps/bit
     *
     * Errors:
     *   NONE
     *
     */
    double getGres(void);

    /*!
     * Description: 
     *   Returns the Accelerometer resolution in g/bit base on the
     *   configuration of the device. 
     *
     * Arguments:
     *   NONE
     *
     * Returns:
     *    resolution in g/bit
     *
     * Errors:
     *    NONE
     *
     */
    double getAres(void);

    /*!
     * Description: 
     *   Read the I2C Temperature register directly and return 
     *   the chip temperature in C
     *
     * Arguments:
     *   NONE
     *
     * Returns:
     *   Temperature of chip in C
     *
     * Errors:
     *   if I2C channel is not open
     *
     */
    double readTempData(void);

    /*!
     * Description: 
     *   reads the 3 axis (X,Y,Z) acceleration in g directly from 
     *   the chip register. The data is stored internally and may be 
     *   accessed via the inline functions below.  
     *
     * Arguments:
     *   NONE
     *
     * Returns:
     *    true on success
     *
     * Errors:
     *    I2C read fail. 
     *
     */
    bool readAccelData(void);

    /*!
     * Description: 
     *   reads the 3 axis (X,Y,Z) Magnetic in uT directly from 
     *   the chip register. The data is stored internally and may be 
     *   accessed via the inline functions below.  
     *
     * Arguments:
     *   results - a user supplied array of dimension 3 for
     *             integer results. 
     *
     * Returns:
     *   true
     *
     * Errors:
     *    I2C read fail. 
     */
    bool readMagData(int16_t *results=NULL);

    /*!
     * Description: 
     *   reads the 3 axis (X,Y,Z) rate in dps directly from 
     *   the chip register. The data is stored internally and may be 
     *   accessed via the inline functions below.
     *
     * Arguments:
     *   NONE
     *
     * Returns:
     *    true on success
     * Errors:
     *    I2C read fail. 
     *
     */
    bool readGyroData(void);

    /*!
     * Description: 
     *   Run an internal test to look at how the internal settings
     *   (factory) look relative to the function. 
     *
     *   This method fools with the chip and this should be exercized 
     *   then exit. 
     *
     * Arguments:
     *   results - a user supplied vector of dimension 6 that
     *             returns Acc (X,Y,Z) and Gyro(X,Y,Z) 
     *             in percent deviation from biases. 
     *
     * Returns:
     *   true - on success. 
     *
     * Errors:
     *    
     */
    bool ICM20948SelfTest(double *results);

    /*!
     * Description: 
     *   Do a magnetic calibration by making a figure 8 with the 
     *   magnetometer for 15 seconds. 
     *
     * Arguments:
     *   bias_dest  - 3 dimensional vector to return the X,Y,Z bias change
     *   scale_dest - 3 dimensional vector to return the X,Y,Z scale change
     *
     * Returns:
     *   NONE
     *
     * Errors:
     *   if I2C channel is not open
     *
     */
     void magCalICM20948(double * bias_dest, double * scale_dest);

    /*! Enable a more friendly way of printing the contents of the class. */
    friend std::ostream& operator<<(std::ostream& output, const ICM20948 &n);

protected:
    /*
     * Always make sure this is still true by searching using
     * sudo i2cdetect -y 1
     * You should see a device 69 if this works. 
     */
    const char *kICMDeviceName = "/dev/i2c-1";
    // File decriptor if opens correctly. 
    int fdI2C;

    // Sub component addresses on the I2C bus.
    uint8_t fIMU_address;
    uint8_t fMag_address;

    // Helper functions for reading and writing to I2C. 
    uint8_t   ReadReg8(uint8_t SlaveAddress, uint8_t Register);
    int       ReadBlock(uint8_t SlaveAddress, unsigned char Register, 
			size_t  size,  void* data);
    bool      WriteReg8(uint8_t SlaveAddress, unsigned char Register, 
			unsigned char value);
    int       ReadWord(uint8_t SlaveAddress, unsigned char Register);
    bool      WriteWord(uint8_t SlaveAddress, uint8_t Register, 
			uint16_t value);

    // Set initial input parameters
    /*
     * Permmisable range limits for the accelerometer
     */
    enum Ascale
    {
      AFS_2G = 0,
      AFS_4G,
      AFS_8G,
      AFS_16G
    };

    /*
     * Permmisable range limits for the Gryo
     */
    enum Gscale {
      GFS_250DPS = 0,
      GFS_500DPS,
      GFS_1000DPS,
      GFS_2000DPS
    };

    // FIXME, not currently used as part of setup
    enum Mscale {
      MFS_14BITS = 0, // 0.6 mG per LSB
      MFS_16BITS      // 0.15 mG per LSB
    };

    enum M_MODE {
      M_8HZ  = 0x02,  // 8 Hz update
      M_100HZ = 0x06  // 100 Hz continuous magnetometer
    };

    /*!
     *  FIXME: Add setter methods for this hard coded stuff
     * Specify sensor full scale +/-
     */
    /*
     * GFS_250DPS
     * GFS_500DPS,
     * GFS_1000DPS,
     * GFS_2000DPS
     */
    uint8_t fGscale; 
    double  fGres; 

    /*
     * Accelerometer
     *  AFS_2G
     *  AFS_4G,
     *  AFS_8G,
     *  AFS_16G
     * 
     */
    uint8_t    fAscale;
    double     fAres;    // resulting resolution for acceleration.

    // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
    uint8_t    fMmode;

private:

    int16_t   itemp;
    uint8_t   fMagMode;   /* Bit field to set magnetic readout mode. */
    time_t    fGMTOffset; 

    /*!
     * Setup the primary registers on the IMU unit.
     * Also open the I2C channel
     */
    bool InitICM20948(void);

    /*!
     * Setup the primary registers on the Mag unit.
     * Also open the I2C channel
     */
    bool InitAK09916(uint8_t MagMode);
};
#endif
