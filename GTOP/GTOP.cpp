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
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/resource.h>
#include <errno.h>
#include <fstream>
#include <cstdlib>
#include <libconfig.h++>
using namespace libconfig;

/// Local Includes.
#include "GTOP.hh"
#include "GTOPdisp.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"
#include "smIPC.hh"

GTOP* GTOP::fGTOP;

const char *SensorName="GPS";     // Sensor name. 
const size_t NVar = 13;

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
GTOP::GTOP(const char* ConfigFile) : CObject()
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
    fConfigFileName = NULL;
    fSerialPortName = NULL;

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

    if(!ConfigFile)
    {
	SetError(ENO_FILE,__LINE__);
	return;
    }

    fConfigFileName = strdup(ConfigFile);
    if(!ReadConfiguration())
    {
	SetError(ECONFIG_READ_FAIL,__LINE__);
	return;
    }

    /* User initialization goes here. ---------------------------- */
    fNMEA_GPS = new NMEA_GPS( fSerialPortName, B9600);

    /*
     * Factory default reset is 9600 8 None 1 
     */
    if (fNMEA_GPS->Error())
    {
	Logger->Log("# %s %s\n","# Failed to open serial:", fSerialPortName); 
	SetError(-1);
	return; /* no sense in continuing. */
    }
    else
    {
        Logger->Log("# %s %s \n", "Opened serial port: ", fSerialPortName);
    }

    /*
     * Setup IPC mechanism. 
     */
#if SM_IPC==1
    fIPC = new GPS_IPC();
    if (fIPC->Error() != 0)
    {
	CLogger::GetThis()->LogError(__FILE__, __LINE__,'W',
				     "Could not initialize IPC.");
	fIPC = NULL;
	SetError(-2); 
	return;
    }
#else
    fIPC = 0;
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
    free(fConfigFileName);
    free(fSerialPortName);

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

	}
	if(fNMEA_GPS->Read())
	{
	    // This is the last message in the read sequence. 
	    if(fNMEA_GPS->LastID() == NMEA_GPS::MESSAGE_VTG)
	    {
		// VTG message is the last in the series. 
		Update();
	    }
	    if (pDisp != NULL)
	    {
		pDisp->Update(fNMEA_GPS);
	    }
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
    // Knots per Hour to Meters per second. 
    // 1852 meters per nautical mile
    // 3600 seconds per hour
    const double KPH2MPS = 1852.0/3600.0;
    float t;
    GGA*  pGGA;
    VTG*  pVTG;
    GSA*  pGSA;
    //RMC*  pRMC;

    // Do IPC
    if (fIPC)
    	fIPC->Update();

    // Any user code or logging belongs here. 
    if (f5Logger!=NULL)
    {
	pGGA = fNMEA_GPS->pGGA();
	t  = pGGA->Seconds();
	t += pGGA->Milli();
	pVTG = fNMEA_GPS->pVTG();

	pGSA = fNMEA_GPS->pGSA();

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
    const char *Names = "Time:Lat:Lon:Z:NSV:PDOP:HDOP:VDOP:TRUE:MAG:SMPS:MODE:CTime";
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
    
    fChangeFile = false;

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

	GPS.lookupValue("Port", Port);
	GPS.lookupValue("Latitude",  fLatitude);
	GPS.lookupValue("Longitude", fLongitude);
	GPS.lookupValue("Altitude",  fAltitude);
	GPS.lookupValue("Reset",     fReset);
	GPS.lookupValue("Debug",     Debug);
	GPS.lookupValue("Display",    fDisplay);
	GPS.lookupValue("Logging",    fLogging);
	GPS.lookupValue("ResetType",  fResetType);

	fSerialPortName = strdup(Port.c_str());
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
		    fConfigFileName);

    }
    catch(const FileIOException &fioex)
    {
	Logger->Log("# I/O error while writing file: %s \n",
		    fConfigFileName);
	delete pCFG;
	return(false);
    }
    delete pCFG;
    SET_DEBUG_STACK;
    return true;
}

