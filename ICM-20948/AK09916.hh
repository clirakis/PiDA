/**
 ******************************************************************
 *
 * Module Name : AK09916.hh
 *
 * Author/Date : C.B. Lirakis / 29-Mar-24
 *
 * Description : 
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 * AK09916.pdf
 *
 *******************************************************************
 */
#ifndef __AK09916_hh_
#define __AK09916_hh_
#   include <inttypes.h>
/*
 * Register map for AK09916
 * WIA2   01H  READ      Device ID         8  
 * ST1    10H  READ      Status 1          8 Data status
 * HXL    11H  READ      Measurement data  8 X-axis data
 * HXH    12H  READ                     8
 * HYL    13H                           8 Y-axis data
 * HYH    14H                           8
 * HZL    15H                           8 Z-axis data
 * HZH    16H                           8
 * ST2    18H READ       Status 2       8 Data status
 * CNTL2  31H READ/WRITE Control 2      8 Control Settings
 * CNTL3  32H READ/WRITE Control 3      8 Control Settings
 * TS1    33H READ/WRITE Test           8 DO NOT ACCESS
 * TS2    34H READ/WRITE Test           8 DO NOT ACCESS
 *
 * Descriptions
 *
 * ADDR  REGISTER NAME D7   D6    D5    D4    D3    D2    D1    D0  
 *              READ-ONLY REGISTER
 *  01H  WIA2          0    0     0     0     1     0     0     1
 *  10H  ST1           0    0     0     0     0     0     DOR   DRDY
 *  11H  HXL           HX7  HX6   HX5   HX4   HX3   HX2   HX1   HX0
 *  12H  HXH           HX15 HX14  HX13  HX12  HX11  HX10  HX9   HX8
 *  13H  HYL           HY7  HY6   HY5   HY4   HY3   HY2   HY1   HY0
 *  14H  HYH           HY15 HY14  HY13  HY12  HY11  HY10  HY9   HY8
 *  15H  HZL           HZ7  HZ6   HZ5   HZ4   HZ3   HZ2   HZ1   HZ0
 *  16H  HZH           HZ15 HZ14  HZ13  HZ12  HZ11  HZ10  HZ9   HZ8
 *  18H  ST2           0    RSV30 RSV29 RSV28 HOFL  0     0     0
 *
 *               WRITE/READ REGISTER
 *  31H  CNTL2         0    0     0     MODE4 MODE3 MODE2 MODE1 MODE0
 *  32H  CNTL3         0    0     0     0     0     0     0     SRST
 *  33H  TS1--------
 *  34H  TS2
 *
 * WIA Device ID
 * ST1 Status 1 DOR -Data overrun,used in read, DRDY - data ready. 
 * HXL to HXZ part of read
 * CNTL2 
 *      MODES
 *      00000 Power-down mode
 *      00001 Single measurement mode
 *      00010 Continuous measurement mode 1
 *      00100 Continuous measurement mode 2
 *      00110 Continuous measurement mode 3
 *      01000 Continuous measurement mode 4
 *      10000 Self-test mode
 *
 * CNTL3
 *  SRST - 0 normal, 1 -reset
 */

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
#define AK09916_CNTL3           0x32  // soft reset write 1.

class AK09916 {
public:

    /*! 
     * When in single measurement mode, changes to power down mode on 
     *    completion and DRDY is set to 1 
     *
     * modes 2,4,6 and 8 stop when power down is set. DRDY bit is set each 
     *    time data is ready. 
     */
    enum READOUT_MODE {
	kPOWER_DOWN  = 0x00,
	kSINGLE_MEAS = 0x01,   // Single measurement
	kM_10HZ      = 0x02,
	kM_20HZ      = 0x04,
	kM_50HZ      = 0x06,
	kM_100HZ     = 0x08,
	kSELF_TEST   = 0x10,
    };


    /// Default Constructor
    AK09916(uint8_t Address, READOUT_MODE Mode);
    /// Default destructor
    ~AK09916(void);
    /// AK09916 function
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
    /*!
     * Magnetic sensor resolution
     * Sections: 2.3, 
     * Earth magnetic field ~(25-65 uT)
     * 4912.0 uT max scale, can go +/-
     * 16 bits bipolar.
     * The code I stole this from had 10.0 multiplier, I don't know why. 
     * Check cal. 
     * 0.15mG per LSB fixed
     */
    const double kMagRes = (4912.0/32767.0);

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
     *   reads the 3 axis (X,Y,Z) Magnetic in uT directly from 
     *   the chip register. integer read only   
     *
     * Arguments:
     *   results - a user supplied array of dimension 3 for
     *             integer results. Multiply by kMagRes to convert
     *             to uT. 
     * Returns:
     *   true
     *
     * Errors:
     *    I2C read fail. 
     */
    bool Read(int16_t *results);

    /* Same as above but does conversion. */
    bool DRead(double *results); 
    /*
     * convert integer result to double applying scaling factor
     * as well. 
     */
    void Convert(int16_t *intArray, double *result);

    inline bool Error(void) {return fError;};

    inline int32_t Address(void) const {return fMagAddress;};

    inline double MagField(uint8_t i) 
	{if(i<3) return fMag[i]; else return 0.0;};

    /*
     * Fleshing out more detail here.
     * Read the register ID.  
     */
    uint8_t DeviceID(void);

    void    SoftReset(void);

    /*
     * Perform self test see 9.4.4 of manual. 
     * data is a vector 3 deep provided by the user. 
     */
    bool    SelfTest(int16_t *data);

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
     bool Calibrate(double * bias_dest, double * scale_dest);

private:
    uint8_t    fMagAddress;
    /*!
     * Readout mode - set in CNTL2 to one of these values. 
     * 
     *      00000 - Power Down mode, stop acquisition
     *      00001 - Single Measurement Mode, one shot
     *      00010 - Continious measurement mode 1  10Hz
     *      00100 - Continious measurement mode 2  20Hz
     *      00110 - Continious measurement mode 3  50Hz
     *      01000 - Continious measurement mode 4 100Hz
     *      10000 - Self test mode, internal mag field is generated!!
     */
    uint8_t    fMmode;     // Measurement mode, see above. 
    bool       fError;
    bool       fMagRead;   // Read of magnetic data success. 
    double     fMag[3];    // resulting magnetic field, converted
};
#endif
