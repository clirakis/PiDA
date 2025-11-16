/**
 ******************************************************************
 *
 * Module Name : GTOP.cpp
 *
 * Author/Date : C.B. Lirakis / 19-Feb-22
 *
 * Description : GTOP, wrap all the logging, configuration file read
 *               and population of IPC here
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions : 
 * 17-Dec-23    CBL The place where we are getting time
 *              is always zero. 
 *              Adding in system time as a parameter as well. 
 * 
 * 20-Dec-23    Never changed filenames. 
 * 10-Mar-24    Added in GPS-pc time delta. 
 * 24-Mar-24    Added in TOD 
 * 08-Sep-25    added in ability to prompt change log file names
 * 15-Nov-25    There have been some upgrades in the general NMEA_LIB
 * 
 * Classification : Unclassified
 *
 * References : 
 *     https://cdn-shop.adafruit.com/datasheets/GlobalTop+MT3339+PC+Tool+Operation+Manual+v1.1.pdf
 *     https://www.adafruit.com/product/790
 *     GlobalTop+MT3339+PC+Tool+Operation+Manual+v1.1.pdf
 * 
 *
 *
 *******************************************************************
 */  
// System includes.
#include <iostream>
using namespace std;

#include <string>
#include <cstring>
#include <cmath>
#include <csignal>
#include <libconfig.h++>
using namespace libconfig;

/// Local Includes.
#include "GTOP.hh"
#include "GTOPdisp.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"
#include "smIPC.hh"
#include "EventCounter.hh"
#include "serial.h"

GTOP* GTOP::fGTOP;

const char *SensorName="GPS";     // Sensor name. 
const size_t NVar = 17;
const size_t kMAXCHARCOUNT = 256;
/**
 ******************************************************************
 *
 * Function Name : GTOP constructor
 *
 * Description : initialize CObject variables and Lassen setup. 
 *
 * Inputs : currently none. 
 *
 * Returns : none
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
GTOP::GTOP(const string& ConfigFile) : CObject()
{
    CLogger *Logger = CLogger::GetThis();

    /* Store the this pointer. */
    fGTOP = this;

    SetName("GTOP");
    SetError(); // No error.

    /* Serial port is not yet open. */
    fNMEA_GPS  = NULL;
    fIPC       = NULL;
    fn         = NULL;
    f5Logger   = NULL;
    fConfigFileName = ConfigFile;

    /* Set some defaults. */
    fLatitude  = 41.3084;
    fLongitude = -73.893;
    fAltitude  = 88.73;
    fReset     = false;
    fDebug     = 0;
    fLogging   = true;
    fDisplay   = false;
    fResetType = 0;

    fGeoLatitude  = 41.3084;
    fGeoLongitude = -73.893;

    if(ConfigFile.length()<=0)
    {
	SetError(ENO_FILE,__LINE__);
	return;
    }


    if(!ReadConfiguration())
    {
	SetError(ECONFIG_READ_FAIL,__LINE__);
	return;
    }

    /* 
     *User initialization goes here. ---------------------------- 
     * Open serial port and then initialize the NMEA decoding package. 
     */

    /*
     * Factory default reset is 9600 8 None 1 
     */
    if (SerialOpen( fSerialPortName.c_str(), B9600)<0)
    {
	Logger->Log("# %s %s\n","# Failed to open serial:", 
		    fSerialPortName.c_str()); 
	SetError(-1);
	return; /* no sense in continuing. */
    }
    else
    {
        Logger->Log("# %s %s \n", "Opened serial port: ", fSerialPortName);
	fNMEA_GPS = new NMEA_GPS();
	fCurrentLine.str("");
    }

    /*
     * Setup IPC mechanism. 
     */
#if SM_IPC==1
    fIPC = new GPS_IPC();
    if (fIPC->Error() != 0)
    {
	Logger->LogError(__FILE__, __LINE__,'W',
				     "Could not initialize IPC.");
	fIPC = NULL;
	SetError(-2); 
	return;
    }
    fEVCounter = new EventCounter(true);
    if(fEVCounter->Error() != 0)
    {
	Logger->LogError(__FILE__, __LINE__,'W',
				     "Could not initialize EventCounter.");
	fEVCounter = NULL;
	SetError(-3); 
	return;
    }
    else
    {
	Logger->LogTime("# Event Counter Started! \n");
    }
#else
    fIPC       = NULL;
    fEVCounter = NULL;
#endif

    if (fLogging)
    {
	fn = new FileName("GTop", "h5", One_Day);
	OpenLogFile();
    }

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : GTOP Destructor
 *
 * Description :
 *          Turn off the run thread
 *          Write out final configuraton, could have changed
 *          Wait for RX thread to show that it is complete
 *          shutdown logger
 *          shutdown IPC mechanism
 *          clean up NMEA_GPS
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 21-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
GTOP::~GTOP(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();

    // Do some other stuff as well. 
    if(!WriteConfiguration())
    {
	SetError(ECONFIG_WRITE_FAIL,__LINE__);
	Logger->LogError(__FILE__,__LINE__, 'W', 
			 "Failed to write config file.\n");
    }

    /* Clean up IPC */
    delete fIPC;
    delete f5Logger;
    f5Logger = NULL;

    delete fNMEA_GPS;

    Logger->LogTime(" GTOP closed.\n");
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : UpdateFileName
 *
 * Description : Flush and close current log file, update the name, 
 *               and reopen.
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void GTOP::UpdateFileName(void)
{
    SET_DEBUG_STACK;
    /*
     * flush and close existing file
     * get a new unique filename
     * reset the timer
     * and go!
     *
     * Check to see that logging is enabled. 
     */
    if(f5Logger)
    {
	// This will close and flush the existing logfile. 
	delete f5Logger;
	f5Logger = NULL;
	// Now reopen
	OpenLogFile();
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Read
 *
 * Description : Read a character at a time.
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
bool GTOP::Read(void)
{
    SET_DEBUG_STACK;
    const struct timespec sleeptime = {0L, 100000000L};
    bool rv = false;

    // Loop over the serial port until we get a full line. 
    char c = 0;
    size_t n = read(GetSerial_fd(), &c, 1);
    if (n == 0)
    {
	nanosleep( &sleeptime, NULL);
	rv = false;
    }
    else if (c == '\n') 
    {
	// Represents an end of line. Null terminate then decode. 
	// go ahead and decode what we have.
	fNMEA_GPS->parse(fCurrentLine.str().c_str());
	rv = true;
    }
    else
    {
	/// buffer overflow situation. 
	fCurrentLine << c;
	if (fCurrentLine.str().length() >= kMAXCHARCOUNT)
	    rv=false;
    }

    SET_DEBUG_STACK;
    return rv;
}
/**
 ******************************************************************
 *
 * Function Name : Do
 *
 * Description : Doesn't doo much as you can see. 
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
void GTOP::Do(void)
{
    //const struct timespec sleeptime = {0L, 100000000L};
    SET_DEBUG_STACK;
    CLogger      *Logger = CLogger::GetThis();
    GTOP_Display *pDisp  = GTOP_Display::GetThis();

    fRun = true;
    while( fRun)
    {
	/* Check to see if the logging interval has rolled over. */
	if (fn->ChangeNames())
	{
	    UpdateFileName();
	}
	// Read serial data until we have a full sentance terminated with a \n
	if(Read())
	{
	    //cout << "DEBUG, read a line: " << fCurrentLine.str() << endl;
	    // This is the last message in the read sequence. 
	    if(fNMEA_GPS->LastID() == NMEA_GPS::kMESSAGE_VTG)
	    {
		// VTG message is the last in the series. 
		Update();
	    }
	    if (pDisp != NULL)
	    {
		pDisp->Update(fNMEA_GPS, fCurrentLine.str());
	    }
	    // reset the stream
	    fCurrentLine.str("");
	}
	//nanosleep( &sleeptime, NULL);
    } // End of run do loop. 
    Logger->LogTime(" Loop terminated. \n");
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Update
 *
 * Description : write data to IPC and HDF5 file. 
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
void GTOP::Update(void)
{
    SET_DEBUG_STACK;
    /*
     * Knots per Hour to Meters per second. 
     * 1852 meters per nautical mile
     * 3600 seconds per hour
     */
    const double KPH2MPS = 1852.0/3600.0;
    GGA*  pGGA;
    VTG*  pVTG;
    GSA*  pGSA;
    RMC*  pRMC;
    uint32_t Count; 
    uint32_t idt;
    double   dt = 0.0;

    // Do IPC
    if (fIPC)
    	fIPC->Update();

    if(fEVCounter)
    {
	fEVCounter->Increment();
	Count = fEVCounter->Count();
    }


    // Any user code or logging belongs here. 
    if (f5Logger!=NULL)
    {
	pRMC = fNMEA_GPS->pRMC();
	pGGA = fNMEA_GPS->pGGA();
	struct timespec PCTime = pGGA->PCTime();
	struct tm *tmnow = localtime(&PCTime.tv_sec);
	double sec = tmnow->tm_sec + tmnow->tm_min*60.0 + 
	    tmnow->tm_hour*3600.0;

	idt      = pGGA->Seconds();
	if (idt > PCTime.tv_sec)
	{
	    idt -= PCTime.tv_sec;
	    dt   = pGGA->Milli() - 1.0e-9 * (double)PCTime.tv_nsec;
	    dt  += (double) idt;
	}
	else
	{
	    idt  = PCTime.tv_sec - idt;
	    dt   = 1.0e-9 * (double)PCTime.tv_nsec -  pGGA->Milli();
	    dt  += (double) idt;
	}
	dt       -= timezone;

	pVTG = fNMEA_GPS->pVTG();

	pGSA = fNMEA_GPS->pGSA();
	double t = pGGA->Seconds() + pGGA->Milli();
	f5Logger->FillInternalVector(t, 0);
	f5Logger->FillInternalVector(pGGA->Latitude()*RadToDeg, 1);
	f5Logger->FillInternalVector(pGGA->Longitude()*RadToDeg, 2);
	f5Logger->FillInternalVector(pGGA->Altitude(), 3);
	f5Logger->FillInternalVector((int)pGGA->Satellites(), 4);
	f5Logger->FillInternalVector(pGSA->PDOP(), 5);
	f5Logger->FillInternalVector(pGSA->HDOP(), 6);
	f5Logger->FillInternalVector(pGSA->VDOP(), 7);
	f5Logger->FillInternalVector(pVTG->True(), 8);
	f5Logger->FillInternalVector(pVTG->Mag(), 9);
	f5Logger->FillInternalVector(pVTG->KPH()*KPH2MPS,10);
	f5Logger->FillInternalVector(pVTG->Mode(),11);

	time_t now;
	time(&now);

	f5Logger->FillInternalVector(now, 12);
	f5Logger->FillInternalVector(Count, 13);
	f5Logger->FillInternalVector(dt,14);
	f5Logger->FillInternalVector(pRMC->Delta(), 15);
	f5Logger->FillInternalVector(sec, 16);

	f5Logger->Fill();
    }
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : OpenLogFile
 *
 * Description : Open and manage the HDF5 log file
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool GTOP::OpenLogFile(void)
{
    SET_DEBUG_STACK;
//    const char *Names = "Time:Lat:Lon:Z:NSV:PDOP:HDOP:VDOP:TDOP:VE:VN:VZ";
    const char *Names = "Time:Lat:Lon:Z:NSV:PDOP:HDOP:VDOP:TRUE:MAG:SMPS:MODE:CTime:EVCount:PCDT:RMCDT:TOD";
    /*
     *
     *  0) Time - Seconds since unix epoch from GGA message
     *  1) Lat  - degrees
     *  2) Lon  - degrees
     *  3) Z    - meters
     *  4) NSV  - Number of satelites in view
     *  5) PDOP
     *  6) HDOP
     *  7) VDOP
     *  8) TRUE - compas true north
     *  9) MAG  - magnetic north
     * 10) SMPS - Speed Meters per second
     * 11) MODE - Velocity mode : Autonomous, Differential, Estimated
     * 12) CTime - computer time seconds since unix epoch
     * 13) EVCount - event count
     * 14) PCDT  - DT between arrival of GGA message computer clock time and
     *             GGA time
     * 15) RMC DT - same but for RMC message
     * 16) TOD - Time of Day
     */
    CLogger *pLogger  = CLogger::GetThis();
    /* Give me a file name.  */
    const char* name  = fn->GetUniqueName();
    fn->NewUpdateTime();
    SET_DEBUG_STACK;

    f5Logger = new H5Logger(name,"GTop GPS Dataset", NVar, false);
    if (f5Logger->CheckError())
    {
	pLogger->Log("# Failed to open H5 log file: %s\n", name);
	delete f5Logger;
	f5Logger = NULL;
	return false;
    }
    f5Logger->WriteDataTags(Names);

    /* Log that this was done in the local text log file. */
    time_t now;
    char   msg[64];
    SET_DEBUG_STACK;
    time(&now);
    strftime (msg, sizeof(msg), "%m-%d-%y %H:%M:%S", gmtime(&now));
    pLogger->Log("# changed file name %s at %s\n", name, msg);

    /*
     * If the IPC is realized, put the current filename into it.
     */ 
     if (fIPC)
     {
 	fIPC->UpdateFilename(name);
     }

    return true;
}
/**
 ******************************************************************
 *
 * Function Name : ReadConfiguration
 *
 * Description : Open read the configuration file. 
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool GTOP::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    /*
     * Open the configuragtion file. 
     */
    try{
	pCFG->readFile("gtop.cfg");
    }
    catch( const FileIOException &fioex)
    {
	Logger->LogError(__FILE__,__LINE__,'F',
			 "I/O error while reading configuration file.\n");
	return false;
    }
    catch (const ParseException &pex)
    {
	Logger->Log("# Parse error at: %s : %d - %s\n",
		    pex.getFile(), pex.getLine(), pex.getError());
	return false;
    }


    /*
     * Start at the top. 
     */
    const Setting& root = pCFG->getRoot();

    // Output a list of all books in the inventory.
    try
    {
	/*
	 * index into group GPS
	 */
	const Setting &GPS = root["GPS"];
	string Port;
	int    Debug;

	GPS.lookupValue("Port", fSerialPortName);
	GPS.lookupValue("Latitude",  fLatitude);
	GPS.lookupValue("Longitude", fLongitude);
	GPS.lookupValue("Altitude",  fAltitude);
	GPS.lookupValue("Reset",     fReset);
	GPS.lookupValue("Debug",     Debug);
	GPS.lookupValue("Display",   fDisplay);
	GPS.lookupValue("Logging",   fLogging);
	GPS.lookupValue("ResetType", fResetType);

	SetDebug(Debug);

	if (fReset)
	{
	    Logger->Log("# Reset is called for.\n");
	}
    }
    catch(const SettingNotFoundException &nfex)
    {
	// Ignore.
    }

    // Output a list of all movies in the inventory.
    try
    {
	/*
	 * index into group Geodetic
	 */
	const Setting &Geodetic = root["Geodetic"];

	Geodetic.lookupValue("Latitude",  fGeoLatitude);
	Geodetic.lookupValue("Longitude", fGeoLongitude);

    }
    catch(const SettingNotFoundException &nfex)
    {
	// Ignore.
    }
    delete pCFG;
    pCFG = 0;
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : WriteConfigurationFile
 *
 * Description : Write out final configuration
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool GTOP::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    Setting &root = pCFG->getRoot();

    // Add some settings to the configuration.
    Setting &GPS = root.add("GPS", Setting::TypeGroup);
    Setting &Geodetic = root.add("Geodetic", Setting::TypeGroup);

    GPS.add("Port",      Setting::TypeString)  = fSerialPortName;
    GPS.add("Latitude",  Setting::TypeFloat)   = fLatitude;
    GPS.add("Longitude", Setting::TypeFloat)   = fLongitude;
    GPS.add("Altitude",  Setting::TypeFloat)   = fAltitude;

    // Reset both the request to reset and the debug parameter. 
    GPS.add("Reset",     Setting::TypeBoolean) = false;
    GPS.add("Debug",     Setting::TypeInt)     = 0;
    GPS.add("Display",   Setting::TypeBoolean) = fDisplay;
    GPS.add("Logging",   Setting::TypeBoolean) = fLogging;
    GPS.add("ResetType", Setting::TypeInt)     = fResetType;

    // These are somewhat residual. 
    Geodetic.add("Latitude",  Setting::TypeFloat) = fGeoLatitude;
    Geodetic.add("Longitude", Setting::TypeFloat) = fGeoLongitude;

    // Write out the new configuration.
    try
    {
	pCFG->writeFile(fConfigFileName);
	Logger->LogTime(" New configuration successfully written to: %s\n",
			fConfigFileName.c_str());

    }
    catch(const FileIOException &fioex)
    {
	Logger->Log("# I/O error while writing file: %s \n",
		    fConfigFileName.c_str());
	delete pCFG;
	return(false);
    }
    delete pCFG;
    SET_DEBUG_STACK;
    return true;
}

