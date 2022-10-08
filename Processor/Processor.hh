/**
 ******************************************************************
 *
 * Module Name : Processor.hh
 *
 * Author/Date : C.B. Lirakis / 22-Feb-22
 *
 * Description : Processor, fuse data from multiple data sources. 
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 *******************************************************************
 */
#ifndef __PROCESSOR_hh_
#define __PROCESSOR_hh_
#  include "CObject.hh" // Base class with all kinds of intermediate
#  include "H5Logger.hh"
#  include "filename.hh"
#  include "NMEA_GPS.hh"
#  include "Geodetic.hh"
#  include "smIPC_GPS.hh"
#  include "smIPC_IMU.hh"

class Processor : public CObject
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
    Processor(const char *ConfigFile);

    /**
     * Destructor for Processor
     */
    ~Processor(void);

    /*! Access the This pointer. */
    static Processor* GetThis(void) {return fProcessor;};

    /**
     * Main Module DO
     * 
     */
    void Do(void);

    /**
     * Tell the program to stop. 
     */
    void Stop(void) {fRun=false;};

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
 
private:
    const uint8_t NVar = 14;

    /* Log the data */
    void Update(void);

    /** Run the program */
    bool fRun;

    /** Parameters to initialize the Geodetic system */
    double fLatDegrees0, fLonDegrees0;

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
     * Configuration file name. 
     */
    char        *fConfigFileName;

    /* Collection of configuration parameters. ===================== */
    bool        fLogging;       /*! Turn logging on. */


    /** connect to shared memory */
    GPS_IPC     *fGPS;

    IMU_IPC     *fIMU;

    /** Make geodetic projections */
    Geodetic    *fGeo; 

    /* Private functions. ==============================  */

    /*!
     * Open the data logger. 
     */
    bool OpenLogFile(void);

    /*!
     * Read the configuration file. 
     */
    bool ReadConfiguration(void);
    /*!
     * Write the configuration file. 
     */
    bool WriteConfiguration(void);



    /*! The static 'this' pointer. */
    static Processor *fProcessor;
};
#endif
