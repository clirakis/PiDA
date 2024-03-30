/**
 ******************************************************************
 *
 * Module Name : IMU.cpp
 *
 * Author/Date : C.B. Lirakis / 24-Feb-22
 *
 * Description : IMU control entry points
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions : 
 * 20-Dec-23   CBL   was not changing filenames on the chosen interval. 
 *
 * Classification : Unclassified
 *
 * References : 
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
#include "IMU.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"

#define SM_IPC 1

IMU* IMU::fIMU;

/**
 ******************************************************************
 *
 * Function Name : IMU constructor
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
IMU::IMU(const char* ConfigFile) : CObject()
{
    CLogger *Logger = CLogger::GetThis();

    /* Store the this pointer. */
    fIMU = this;
    SetName("IMU");
    SetError(); // No error.

    fRun         = true;
    fICM20948    = NULL;
    fIPC         = NULL;
    fSampleRate  = 1;     // 1 Hz
    fNSamples    = 10;    // 10 samples

    /* 
     * Set defaults for configuration file. 
     */
    fLogging = true;

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
	fn = new FileName("IMU", "h5", One_Day);
	OpenLogFile();
    }

    /*
     * Setup IPC mechanism. 
     */
#if SM_IPC==1
    fIPC = new IMU_IPC();
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

    Logger->Log("# IMU constructed.\n");

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : IMU Destructor
 *
 * Description :
 *    delete the I2C connection with the devices.
 *    Write final configuration file. 
 *    Finalize data logger
 *    Finalize IPC mechanism
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
IMU::~IMU(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();

    delete fICM20948;

    // Do some other stuff as well. 
    if(!WriteConfiguration())
    {
	SetError(ECONFIG_WRITE_FAIL,__LINE__);
	Logger->LogError(__FILE__,__LINE__, 'W', 
			 "Failed to write config file.\n");
    }
    free(fConfigFileName);

    /* Clean up */
    delete f5Logger;
    f5Logger = NULL;

    delete fIPC;

    // Make sure all file streams are closed
    Logger->Log("# IMU closed.\n");
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Do
 *
 * Description : The main loop for the program, do the operations. 
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
void IMU::Do(void)
{
    SET_DEBUG_STACK;
    fRun = true;
    int32_t i = 0;

    /*
     * if fNSamples is negative, means infinite.
     */
    do 
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

	/* Read everything. */
	fICM20948->Read();
	if (fDebug>0)
	    cout << *fICM20948;
	if (fn) 
	    Update();
	nanosleep(&fSampleTime, NULL);
	if (fNSamples>0)
	{
	    i++;
	    if (i>fNSamples) fRun = false;
	}
	
    } while (fRun);
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Update
 *
 * Description :
 *    Update the data in the IPC if active
 *    If Logger is enabled, log the data. 
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
void IMU::Update(void)
{
    SET_DEBUG_STACK;
    GGA *pGGA = NULL;

    // Do IPC
    if (fIPC)
    {
    	fIPC->Update();
	pGGA = fIPC->GetPosition();
    }

    // Any user code or logging belongs here. 
    if (f5Logger!=NULL)
    {
	struct timespec ts = fICM20948->ReadTime();
	double t = (double) ts.tv_sec + (double)ts.tv_nsec*1.0e-9;
	f5Logger->FillInternalVector( t, 0);
	f5Logger->FillInternalVector(  fICM20948->AccX(),   1);
	f5Logger->FillInternalVector(  fICM20948->AccY(),   2);
	f5Logger->FillInternalVector(  fICM20948->AccZ(),   3);
	f5Logger->FillInternalVector(  fICM20948->GyroX(),  4);
	f5Logger->FillInternalVector(  fICM20948->GyroY(),  5);
	f5Logger->FillInternalVector(  fICM20948->GyroZ(),  6);
	f5Logger->FillInternalVector(  fICM20948->MagX(),   7);
	f5Logger->FillInternalVector(  fICM20948->MagY(),   8);
	f5Logger->FillInternalVector(  fICM20948->MagZ(),   9);
	f5Logger->FillInternalVector(  fICM20948->Temp(),  10);
	if (pGGA)
	{
	    f5Logger->FillInternalVector(pGGA->Latitude()*RadToDeg, 11);
	    f5Logger->FillInternalVector(pGGA->Longitude()*RadToDeg, 12);
	    f5Logger->FillInternalVector(pGGA->Altitude(), 13);
	    f5Logger->FillInternalVector(pGGA->UTC(), 14);
	}
	else
	{
	    f5Logger->FillInternalVector(0.0, 11);
	    f5Logger->FillInternalVector(0.0, 12);
	    f5Logger->FillInternalVector(0.0, 13);
	    f5Logger->FillInternalVector(0.0, 14);
	}

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
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IMU::OpenLogFile(void)
{
    SET_DEBUG_STACK;

    // USER TO FILL IN.
    const char *Names = "Time:Ax:Ay:Az:Rx:Ry:Rz:Mx:My:Mz:T:Lat:Lon:Z:UTC";
    CLogger *pLogger = CLogger::GetThis();

    /* Give me a file name.  */
    const char* name = fn->GetUniqueName();
    fn->NewUpdateTime();
    SET_DEBUG_STACK;

    f5Logger = new H5Logger(name,"IMU Dataset", kNVar, false);
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

int32_t IMU::Address(void) 
{
    int32_t rv = 0;
    if (fICM20948)
    {
	rv = fICM20948->Address();
    }
    return rv;
}

/**
 * selftest
 */
bool IMU::SelfTest(double *rv) 
{
    SET_DEBUG_STACK;
    bool rc = false;
    if (fICM20948)
    {
	rc = fICM20948->ICM20948SelfTest(rv);
    }
    return rc;
}

/**
 ******************************************************************
 *
 * Function Name : ReadConfiguration
 *
 * Description : Open read the configuration file. 
 * Based on the information in the configuration, initialize the correct
 * components. 
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IMU::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();
    int32_t MagAddress;
    int32_t IMUAddress; /*! I2C device address for Acc/Gyro. */
    /*
     * Open the configuragtion file. 
     */
    try{
	pCFG->readFile(fConfigFileName);
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

    // USER TO FILL IN
    // Output a list of all books in the inventory.
    try
    {
	/*
	 * index into group IMU
	 */
	const Setting &MM = root["IMU"];
	MM.lookupValue("Logging",       fLogging);
	MM.lookupValue("DebugLevel",    fDebug);
	MM.lookupValue("IMUAddress",    IMUAddress);
	MM.lookupValue("MagAddress",    MagAddress);
	MM.lookupValue("SampleRate",    fSampleRate);
	MM.lookupValue("NumberSamples", fNSamples);
	
	double ival;
	double Period = 1.0/((double) fSampleRate);
	double frac = modf( Period, &ival);
	fSampleTime.tv_sec  = (unsigned long) ival; 
	fSampleTime.tv_nsec = (unsigned long) floor(frac*1.0e9);

    }
    catch(const SettingNotFoundException &nfex)
    {
	// Ignore.
    }

    delete pCFG;
    pCFG = 0;

    fICM20948 = new ICM20948(IMUAddress, MagAddress);
    if (fICM20948->CheckError())
    {
	Logger->Log("# FAIL ON ICM20948 setup.\n");
	SetError(-1,__LINE__);
	delete fICM20948;
	fICM20948 = NULL;
	return false;
    }

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
 * Unit Tested on:  27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IMU::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    ClearError(__LINE__);
    CLogger *Logger = CLogger::GetThis();
    Config *pCFG    = new Config();
    Setting &root   = pCFG->getRoot();
    int32_t IMUAddress;
    int32_t MagAddress;

    if (fICM20948)
    {
	IMUAddress = fICM20948->Address();
	MagAddress = fICM20948->MagAddress();
    }
    else
    {
	IMUAddress  = 0x69;  // Defaults. 
	MagAddress  = 0x0C;
    }
    // USER TO FILL IN
    // Add some settings to the configuration.
    Setting &MM = root.add("IMU", Setting::TypeGroup);
    MM.add("DebugLevel", Setting::TypeInt)     = (int) fDebug;
    MM.add("Logging",    Setting::TypeBoolean) = fLogging;
    MM.add("IMUAddress", Setting::TypeInt)     = (int) IMUAddress;
    MM.add("MagAddress", Setting::TypeInt)     = (int) MagAddress;
    MM.add("SampleRate", Setting::TypeInt)     = (int) fSampleRate;
    MM.add("NumberSamples", Setting::TypeInt)  = (int) fNSamples;

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
