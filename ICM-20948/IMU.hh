/**
 ******************************************************************
 *
 * Module Name : IMU.hh
 *
 * Author/Date : C.B. Lirakis / 22-Feb-22
 *
 * Description : This is meant as a generic wrapper for any IMU in use. 
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions :
 * 30-Mar-24 moved I2C and mag sensor to this level. 
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 *******************************************************************
 */
#ifndef __IMU_hh_
#define __IMU_hh_
#  include "CObject.hh" // Base class with all kinds of intermediate
#  include "H5Logger.hh"
#  include "filename.hh"
#  include "ICM-20948.hh"
#  include "smIPC.hh"

//class H5Logger;

class IMU : public CObject
{
public:

    /** 
     * Build on CObject error codes. 
     */
    enum {ENO_FILE=1, ECONFIG_READ_FAIL, ECONFIG_WRITE_FAIL};
    /**
     * Constructor the lassen SK8 subsystem.
     * All inputs are in configuration file. 
     */
    IMU(const char *ConfigFile);

    /**
     * Destructor for SK8. 
     */
    ~IMU(void);


    /**
     * Main Module DO
     * 
     */
    void Do(void);

    /**
     * Tell the program to stop. 
     */
    inline void Stop(void) {fRun=false;};

    /**
     * IMU address
     */
    int32_t Address(void);

    /**
     * selftest
     */
    bool SelfTest(double *rv);

//     inline void MagCal(double *b, double *s) 
// 	{return fICM20948->magCalICM20948(b, s);};

    /*! Access the This pointer. */
    static IMU* GetThis(void) {return fIMU;};


    /**
     * Control bits - control verbosity of output
     */
    static const unsigned int kVerboseBasic    = 0x0001;
    static const unsigned int kVerboseMSG      = 0x0002;
    static const unsigned int kVerboseFrame    = 0x0010;
    static const unsigned int kVerbosePosition = 0x0020;
    static const unsigned int kVerboseHexDump  = 0x0040;
    static const unsigned int kVerboseCharDump = 0x0080;
    static const unsigned int kVerboseMax      = 0x8000;
 
protected:
    const size_t kNVar = 15;

private:

    bool fRun;
    /*!
     * Tool to manage the file name. 
     */
    FileName*    fn;          /*! File nameing utilities. */
    PreciseTime* fTimer;      /*! */
    bool         fChangeFile; /*! Tell the system to change the file name. */

    /*!
     * Logging tool, log data to HDF5 file.  
     */
    H5Logger    *f5Logger;

    /*!
     * IPC pointer. FIXME
     */
    IMU_IPC      *fIPC;

    /*! 
     * Configuration file name. 
     */
    char        *fConfigFileName;

    /*! Collection of configuration parameters. */
    bool            fLogging;    /*! Turn logging on. */
    uint32_t        fSampleRate; /*! Integer Hz. */
    int32_t         fNSamples;   /*! Number of Samples to take before quit. */
    struct timespec fSampleTime; /*! Time for the above. */


    /*! The sensor itself. */
    ICM20948    *fICM20948;


    /* Private functions. ==============================  */

    /*!
     * Open the data logger. 
     */
    bool OpenLogFile(void);

    /*!
     * Update - Update all the data, log the data, fill IPC ...
     */
    void Update(void);

    /*!
     * Read the configuration file. 
     */
    bool ReadConfiguration(void);
    /*!
     * Write the configuration file. 
     */
    bool WriteConfiguration(void);

    /*! The static 'this' pointer. */
    static IMU *fIMU;

};
#endif
