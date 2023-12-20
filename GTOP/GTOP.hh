/**
 ******************************************************************
 *
 * Module Name : GTOP.hh
 *
 * Author/Date : C.B. Lirakis / 19-Feb-22
 *
 * Description : GTOP GPS receiver running in NMEA mode
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 * Bought from Adafruit.com awhile ago. I think there are newer revs. 
 * This one is MT3339 
 * https://cdn-shop.adafruit.com/datasheets/GlobalTop+MT3339+PC+Tool+Operation+Manual+v1.1.pdf
 * https://www.adafruit.com/product/790
 *
 *
 *******************************************************************
 */
#ifndef __GTOP_hh_
#define __GTOP_hh_
#  include "CObject.hh" // Base class with all kinds of intermediate
#  include "NMEA_GPS.hh"  // Class definitions for NMEA GPS objects.
#  include "smIPC.hh"
#  include "H5Logger.hh"
#  include "filename.hh"
class EventCounter;

class GTOP : public CObject
{
public:
    /** 
     * Build on CObject error codes. 
     */
    enum {ENO_FILE=1, ECONFIG_READ_FAIL, ECONFIG_WRITE_FAIL};
    /**
     * Constructor the lassen GTOP subsystem.
     * All inputs are in configuration file. 
     */
    GTOP(const char *ConfigFile);

    /**
     * Destructor for GTOP. 
     */
    ~GTOP(void);

    /*! Access the This pointer. */
    static GTOP* GetThis(void) {return fGTOP;};

    /** Gain access to NMEA_GPS pointer */
    NMEA_GPS *GetNMEAGPS(void) {return fNMEA_GPS;};

    /**
     * GTOP_do - loop on the serial port and read data from the NMEA_GPS in
     * NMEA format. Parse the data and put the results in shared memory. 
     */
    void Do(void);

    /**
     * Tell the program to stop. 
     */
    void Stop(void) {fRun=false;};

    /*!
     * Has the display been requested?
     */
    inline bool DisplayOn(void) const {return fDisplay;};

    /*!
     * Write the data to the HDF5 logger if open and the IPC if it
     * exists. 
     */
    void Update(void);

    /**
     * Control bits - control verbosity of output
     */
    static const unsigned int kVerboseBasic    = 0x0001;
    static const unsigned int kVerboseMSG      = 0x0002;
    static const unsigned int kVerboseFrame    = 0x0010;
    static const unsigned int kVerboseMax      = 0x8000;
 
private:

    NMEA_GPS      *fNMEA_GPS;

    /** Pointers to various threads. */
    pthread_t     fRX_thread, fDisplay_thread;

    /** Keep the program loop running. */
    bool          fRun;

    /** Manage shared memory segments */
    GPS_IPC*      fIPC;

    /*!
     * Tool to manage the file name. 
     */
    FileName*    fn;          /*! File nameing utilities. */
    //PreciseTime* fTimer;      /*! */
    bool         fChangeFile; /*! Tell the system to change the file name. */

    /*!
     * Logging tool, log data to HDF5 file.  
     */
    H5Logger    *f5Logger;

    /*! 
     * Configuration file name. 
     */
    char   *fConfigFileName;

    /*!
     * Event counter 
     */
    EventCounter *fEVCounter;

    /* Collection of configuration parameters. */
    /*!
     * Serial port name. 
     */
    char*  fSerialPortName;
    double fLatitude;      /*! Starting Latitude  */
    double fLongitude;     /*! Starting Longitude */
    double fAltitude;      /*! Starting Altitude  */
    bool   fReset;         /*! Reset requested.   */
    double fGeoLatitude;   /*! Geodetic Latitude if needed for projections. */
    double fGeoLongitude;  /*! Geodetic Longitude if needed for projections.*/
    bool   fDisplay;       /*! Turn curses display on. */
    bool   fLogging;       /*! Turn logging on. */
    int    fResetType;     /*! 1 - soft reset, 2 Hard reset */


    /* Private functions. =============================================   */
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
    static GTOP *fGTOP;

};
#endif
