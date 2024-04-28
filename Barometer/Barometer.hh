/**
 ******************************************************************
 *
 * Module Name : Barometer.hh
 *
 * Author/Date : C.B. Lirakis / 24-Apr-24
 *
 * Description : Template for a main class
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
#ifndef __BAROMETER_hh_
#define __BAROMETER_hh_
#  include "CObject.hh" // Base class with all kinds of intermediate

class H5Logger;
class FileName;
class PreciseTime;
class SerialIO;
class BARO_IPC;

class Barometer : public CObject
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
    Barometer(const char *ConfigFile);

    /**
     * Destructor for SK8. 
     */
    ~Barometer(void);

    /*! Access the This pointer. */
    static Barometer* GetThis(void) {return fBarometer;};

    /**
     * Main Module DO
     * 
     */
    void Do(void);

    /**
     * Tell the program to stop. 
     */
    void Stop(void) {fRun=false;};

    inline size_t DataSize(void) {return sizeof(double);};
    inline void * DataPointer(void) {return &fValue;};

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

    bool           fRun;
    /*!
     * Tool to manage the file name. 
     */
    FileName*      fn;          /*! File nameing utilities. */
    PreciseTime*   fTimer;      /*! */
    bool           fChangeFile; /*! Tell the system to change the file name. */

    /*!
     * Logging tool, log data to HDF5 file.  
     */
    H5Logger       *f5Logger;

    /*!
     * IPC pointer.
     */
    BARO_IPC        *fIPC;

    /*! 
     * Configuration file name. 
     */
    char            *fConfigFileName;

    /*!
     * Serial IO to sensor. 
     */
    SerialIO        *fIO;

    char            *fSerialPort;

    /* Collection of configuration parameters. */
    bool            fLogging;       /*! Turn logging on. */

    /*!
     * Data segment. 
     */
    double          fValue;
    struct timespec fSampleTime; 


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
    static Barometer *fBarometer;

};
#endif
