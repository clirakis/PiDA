/**
 ******************************************************************
 *
 * Module Name : Barometer.cpp
 *
 * Author/Date : C.B. Lirakis / 24-Apr-24
 *
 * Description : Lassen control entry points. 
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : 
 *              Atmospheric Instrument Research AIR-DB-2A
 *              digital barometer. 800-1060mb -25 to +50 C
 *              SERIAL: 3F2829
 *              9305081412
 *
 *
 *******************************************************************
 */  
// System includes.
#include <iostream>
using namespace std;

#include <string>
#include <cmath>
#include <csignal>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <libconfig.h++>
using namespace libconfig;

/// Local Includes.
#include "Barometer.hh"
#include "smIPC.hh"
#include "H5Logger.hh"
#include "SerialIO.h"
#include "filename.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"

Barometer* Barometer::fBarometer;


/**
 ******************************************************************
 *
 * Function Name : Barometer constructor
 *
 * Description : initialize CObject variables
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
Barometer::Barometer(const char* ConfigFile) : CObject()
{
    CLogger *Logger = CLogger::GetThis();

    /* Store the this pointer. */
    fBarometer = this;
    SetName("Barometer");
    SetError(); // No error.

    fRun        = true;

    /* 
     * Set defaults for configuration file. 
     */
    fn          = NULL;
    fTimer      = NULL;
    f5Logger    = NULL;
    fLogging    = true;
    fIO         = NULL;
    fIPC        = NULL;
    fIO         = NULL;
    fSerialPort = strdup("/dev/ttyUSB0");

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

    /* USER POST CONFIGURATION STUFF. */

    f5Logger = NULL;
    fn       = NULL;
    if (fLogging)
    {
	fn = new FileName("Baro", "h5", One_Day);
	OpenLogFile();
    }

    fIPC = new BARO_IPC();
    if (fIPC->Error() != 0)
    {
	CLogger::GetThis()->LogError(__FILE__, __LINE__,'W',
				     "Could not initialize IPC.");
	fIPC = NULL;
	return;
    }

    Logger->Log("# Barometer constructed.\n");

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Barometer Destructor
 *
 * Description :
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
Barometer::~Barometer(void)
{
    SET_DEBUG_STACK;
    CLogger *pLogger = CLogger::GetThis();

    delete fIPC;

    // Do some other stuff as well. 
    if(!WriteConfiguration())
    {
	SetError(ECONFIG_WRITE_FAIL,__LINE__);
	pLogger->LogError(__FILE__,__LINE__, 'W', 
			 "Failed to write config file.\n");
    }
    free(fConfigFileName);

    free(fSerialPort);

    /* Clean up */
    delete f5Logger;
    f5Logger = NULL;
    // Make sure all file streams are closed
    pLogger->Log("# Barometer closed.\n");

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Do
 *
 * Description :
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
void Barometer::Do(void)
{
    SET_DEBUG_STACK;
    const struct timespec sleeptime = {0L, 500000000};
    CLogger               *pLogger = CLogger::GetThis();
    char                  data[64];
    char                  *p;
    int                   N;
    double                pressure = 0.0;
    GGA                   *pGGA = NULL;

    fRun = true;
    while(fRun)
    {
	if (fn)
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
	}

	memset(data,0, sizeof(data));
	N = fIO->Read( (unsigned char *)data, sizeof(data));
	if (( p = strchr((char *) data, 0x0D)) != NULL)
	{
	    if (fIPC)
	    {
		fIPC->Update();
		pGGA = fIPC->GetPosition();
	    }

	    *p = 0;
	    // null terminate line
	    pressure = atof(data);
	    clock_gettime(CLOCK_REALTIME, &fSampleTime);
	    double t = (double) fSampleTime.tv_sec + 
		(double)fSampleTime.tv_nsec*1.0e-9;
	    f5Logger->FillInternalVector( t,             0);
	    if (pGGA)
	    {
		f5Logger->FillInternalVector(pGGA->Latitude()*RadToDeg,  1);
		f5Logger->FillInternalVector(pGGA->Longitude()*RadToDeg, 2);
		f5Logger->FillInternalVector(pGGA->Altitude(),           3);
		f5Logger->FillInternalVector(pGGA->UTC(),                4);
	    }
	    else
	    {
		f5Logger->FillInternalVector(0.0, 1);
		f5Logger->FillInternalVector(0.0, 2);
		f5Logger->FillInternalVector(0.0, 3);
		f5Logger->FillInternalVector(0.0, 4);
	    }
	    f5Logger->FillInternalVector(pressure, 5);
	    f5Logger->Fill();

	    //cout << pressure << " " << ctime(&fSampleTime.tv_sec);
	    pLogger->Log("%ld, %f\n", fSampleTime.tv_sec, pressure);
	}
	nanosleep(&sleeptime, NULL);
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
bool Barometer::OpenLogFile(void)
{
    SET_DEBUG_STACK;

    // USER TO FILL IN.
    const uint32_t  NVar = 6;
    const char *Names = "Time:Lat:Lon:Z:UTC:MBAR";
    CLogger *pLogger = CLogger::GetThis();
    /* Give me a file name.  */
    const char* name = fn->GetUniqueName();
    fn->NewUpdateTime();
    SET_DEBUG_STACK;

    f5Logger = new H5Logger(name,"Main Logger Dataset", NVar, false);
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
bool Barometer::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    /*
     * Open the configuragtion file. 
     */
    try{
	pCFG->readFile(fConfigFileName);
    }
    catch( const FileIOException &fioex)
    {
	pLog->LogError(__FILE__,__LINE__,'F',
			 "I/O error while reading configuration file.\n");
	return false;
    }
    catch (const ParseException &pex)
    {
	pLog->Log("# Parse error at: %s : %d - %s\n",
		    pex.getFile(), pex.getLine(), pex.getError());
	return false;
    }


    /*
     * Start at the top. 
     */
    const Setting& root = pCFG->getRoot();

    // USER TO FILL IN
    // Output a list of all books in the inventory.
    try
    {
	int    Debug;
	string Port;
	/*
	 * index into group Barometer
	 */
	const Setting &MM = root["Barometer"];
	MM.lookupValue("Logging",   fLogging);
	MM.lookupValue("Debug",     Debug);
	MM.lookupValue("Port",      Port);
	SetDebug(Debug);

	free(fSerialPort);
	fSerialPort = strdup(Port.c_str());

	// now open the port. 
	fIO = new SerialIO (fSerialPort, B9600,
			    SerialIO::NONE,
			    SerialIO::ModeCanonical,
			    0x0D
	    );
	if (fIO->Error() == SerialIO::NOError)
	{
	    // Get the name of a logging file etc. 
	    pLog->LogTime("Input port: %s\n", fSerialPort);
    }
    else
    {
        pLog->LogTime("Error opening serial port: %s %s\n", 
		      fSerialPort,
		      fIO->Error());
	delete fIO;
	fIO = NULL;
        return false;
    }

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
bool Barometer::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    Setting &root = pCFG->getRoot();

    // USER TO FILL IN
    // Add some settings to the configuration.
    Setting &MM = root.add("Barometer", Setting::TypeGroup);
    MM.add("Debug",     Setting::TypeInt)     = 0;
    MM.add("Logging",   Setting::TypeBoolean)     = true;
    MM.add("Port",      Setting::TypeString)      = fSerialPort;

    // Write out the new configuration.
    try
    {
	pCFG->writeFile(fConfigFileName);
	Logger->Log("# New configuration successfully written to: %s\n",
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
