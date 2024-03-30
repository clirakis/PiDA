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


class AK09916 {
public:
    /// Default Constructor
    AK09916(uint8_t Address, uint8_t Mode);
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
     *   reads the 3 axis (X,Y,Z) Magnetic in uT directly from 
     *   the chip register. The data is stored internally and may be 
     *   accessed via the inline functions below.  
     *
     * Arguments:
     *   results - a user supplied array of dimension 3 for
     *             integer results. Multiply by kMagRes to convert
     *             to uT. 
     *
     * Returns:
     *   true
     *
     * Errors:
     *    I2C read fail. 
     */
    bool Read(int16_t *results);

    void Copy(double *array);

    void Calibrate(double * bias_dest, double * scale_dest);

    inline bool Error(void) {return fError;};

    inline int32_t Address(void) const {return fMagAddress;};

    inline double MagField(uint8_t i) 
	{if(i<3) return fMag[i]; else return 0.0;};


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

private:
    uint8_t    fMagAddress;
    uint8_t    fMagMode;   /* Bit field to set magnetic readout mode. */
    // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
    uint8_t    fMmode;
    bool       fError;
    bool       fMagRead;   // Read of magnetic data success. 
    //uint16_t   fResult;
    double     fMag[3];    // resulting magnetic field
};
#endif
