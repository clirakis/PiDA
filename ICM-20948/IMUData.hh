/**
 ******************************************************************
 *
 * Module Name : IMUData.hh
 *
 * Author/Date : C.B. Lirakis / 27-Feb-22
 *
 * Description : Oganize all IMU data in a generic way
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *     29-Mar-24 Changed fMag to fMagXYZ
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
#ifndef __IMUDATA_hh_
#define __IMUDATA_hh_
#    include <time.h>

class IMUData 
{
public:
    /// Default Constructor
    IMUData(void);
    /// Default destructor
    ~IMUData(void);

    /*! Access the This pointer. */
    static IMUData* GetThis(void) {return fIMUData;};

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


    /* ******************** ACCESS METHODS ******************* */
    // Accelerometer values in g
    inline double AccX(void)  const {return fAcc[0];};
    inline double AccY(void)  const {return fAcc[1];};
    inline double AccZ(void)  const {return fAcc[2];};

    // Gyro values in dps
    inline double GyroX(void) const {return fGyro[0];};
    inline double GyroY(void) const {return fGyro[1];};
    inline double GyroZ(void) const {return fGyro[2];};

    // Magnetic values in uT
    inline double MagX(void)  const {return fMagXYZ[0];};
    inline double MagY(void)  const {return fMagXYZ[1];};
    inline double MagZ(void)  const {return fMagXYZ[2];};

    /* Internal chip temperature in C */
    inline double Temp(void) const {return fTemp;}

    /* Get time of read  on real time clock */
    inline struct timespec ReadTime(void) const {return fReadTime;};


    /* THINGS USED to relay in shared memory. --------------------- */
    /*! Get a pointer to the beginning of the data storage. */
    inline void* DataPointer(void) {return (void*)&fReadTime;};
    /*! Return the overall data size for the structure. */
    inline static size_t DataSize(void) 
	{return (sizeof(struct timespec)+10*sizeof(double)+sizeof(bool));};

    /*! Enable a more friendly way of printing the contents of the class. */
    friend std::ostream& operator<<(std::ostream& output, const IMUData &n);

protected:
    /* DATA SEGMENT */
    struct timespec fReadTime;
    double    fAcc[3];    /*! Values from Accelerometer stored when read */
    double    fMagXYZ[3]; /*! Values from Magnetometer stored when read */
    double    fGyro[3];   /*! Values from Gryo stored when read */
    double    fTemp;      /*! Last temperature read. */

    /*! The static 'this' pointer. */
    static IMUData *fIMUData;
};
#endif
