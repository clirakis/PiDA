/**
 ******************************************************************
 *
 * Module Name : Processor.cpp
 *
 * Author/Date : C.B. Lirakis / 05-Mar-19
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
 * https://hyperrealm.github.io/libconfig/libconfig_manual.html
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
#include "Processor.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"

Processor* Processor::fProcessor;

/**
 ******************************************************************
 *
 * Function Name : Processor constructor
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
Processor::Processor(const char* ConfigFile) : CObject()
{
    CLogger *Logger = CLogger::GetThis();

    /* Store the this pointer. */
    fProcessor = this;
    SetName("Processor");
    SetError(); // No error.
    fGPS = NULL;
    fIMU = NULL;
    fGeo = NULL;
    fLatDegrees0 =  41.3082;
    fLonDegrees0 = -73.893;

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
	fn = new FileName("Processor", "h5", One_Day);
	OpenLogFile();
    }

    /* Connnect to GPS shared memory */
    fGPS = new GPS_IPC();

    fIMU = new IMU_IPC();

    /* Bring up a projection. */
    fGeo = new Geodetic( fLatDegrees0, fLonDegrees0);


    Logger->Log("# Processor constructed.\n");
    fRun = true;

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Processor Destructor
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
Processor::~Processor(void)
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

    /* Clean up */
    delete fGPS;
    fGPS = NULL;
    delete fIMU;
    fIMU = NULL;
    delete fGeo;
    delete f5Logger;
    f5Logger = NULL;

    // Make sure all file streams are closed
    Logger->Log("# Processor closed.\n");
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
void Processor::Do(void)
{
    const struct timespec sleeptime = {0L, 100000000L};
    SET_DEBUG_STACK;
    Point  current, delta;
    double Lat, Lon;
    time_t Seconds;
    double milli, tdelta;
    RMC    *pRMC = fGPS->GetRMC();
    Point xy0 = fGeo->XY0();

    fRun = true;
    while(fRun)
    {
	if(fGPS->Update())
	{
	    Lat = pRMC->Latitude();
	    Lon = pRMC->Longitude();
	    current = fGeo->ToXY( Lon, Lat);
	    delta   = xy0 - current;
	    Seconds = pRMC->Seconds();
	    milli   = pRMC->Milli();
	    cout << " ---------------------------------------" << endl;
	    cout << "Lat/Lon: " 
		 << RadToDeg * Lat 
		 << " " << RadToDeg * Lon
		 << " XYDelta: " << delta
		 << " TDelta: " << pRMC->Delta()
		 << endl;

	
	    if((fIMU!=NULL) & fIMU->Update())
	    {
		IMUData *pData = fIMU->GetIMU();
		struct timespec IMUTime = fIMU->GetIMU()->ReadTime();
		tdelta = milli - ((double) IMUTime.tv_nsec)*1.0e-9;
		tdelta += (double)(Seconds-IMUTime.tv_sec);
		tdelta += pRMC->Delta();
		cout << *pData << endl;
		cout << " DTIME S: " 
		     << Seconds - IMUTime.tv_sec 
		     << " Milli: " << milli
		     << " ns: " << IMUTime.tv_nsec
		     << " tdelta: " << tdelta
		     << endl;
	    }
	    Update();
	}
	nanosleep( &sleeptime, NULL);
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Update
 *
 * Description : Log the data. 
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
void Processor::Update(void)
{
    SET_DEBUG_STACK;

    // Do some graphs, add later. 

    // Any user code or logging belongs here. 
    if (f5Logger!=NULL)
    {
	GGA     *pGGA   = fGPS->GetGGA();
	RMC     *pRMC   = fGPS->GetRMC();
	IMUData *pData  = fIMU->GetIMU();
	double  Seconds = pRMC->Seconds();
	double  milli   = pRMC->Milli();
	struct timespec IMUTime = fIMU->GetIMU()->ReadTime();
	double tdelta = milli - ((double) IMUTime.tv_nsec)*1.0e-9;
	tdelta += (double)(Seconds-IMUTime.tv_sec);
	tdelta += pRMC->Delta();

	f5Logger->FillInternalVector(  pRMC->Seconds(),   0);
	f5Logger->FillInternalVector(  pRMC->Latitude()*RadToDeg, 1);
	f5Logger->FillInternalVector(  pRMC->Longitude()*RadToDeg,2);
	f5Logger->FillInternalVector(  pGGA->Altitude(),          3);

	f5Logger->FillInternalVector(  pData->AccX(),   4);
	f5Logger->FillInternalVector(  pData->AccX(),   5);
	f5Logger->FillInternalVector(  pData->AccY(),   6);
	f5Logger->FillInternalVector(  pData->GyroX(),  7);
	f5Logger->FillInternalVector(  pData->GyroY(),  8);
	f5Logger->FillInternalVector(  pData->GyroZ(),  9);
	f5Logger->FillInternalVector(  pData->MagX(),  10);
	f5Logger->FillInternalVector(  pData->MagY(),  11);
	f5Logger->FillInternalVector(  pData->MagZ(),  12);
	f5Logger->FillInternalVector(  tdelta,         13);

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
bool Processor::OpenLogFile(void)
{
    SET_DEBUG_STACK;

    // USER TO FILL IN.
    const char *Names = "Time:Lat:Lon:Z:Ax:Ay:Az:Rx:Ry:Rz:Mx:My:Mz:TD";
    CLogger *pLogger = CLogger::GetThis();
    /* Give me a file name.  */
    const char* name = fn->GetUniqueName();
    fn->NewUpdateTime();
    SET_DEBUG_STACK;

    f5Logger = new H5Logger(name,"Processor Logger Dataset", NVar, false);
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
bool Processor::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
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
	 * index into group Processor
	 */
	const Setting &MM = root["Processor"];
	MM.lookupValue("Logging",     fLogging);
	MM.lookupValue("Debug",       fDebug);
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

	Geodetic.lookupValue("StartingLat", fLatDegrees0);
	Geodetic.lookupValue("StartingLon", fLonDegrees0);
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
bool Processor::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    Setting &root = pCFG->getRoot();

    // USER TO FILL IN
    // Add some settings to the configuration.
    Setting &MM = root.add("Processor", Setting::TypeGroup);
    MM.add("Debug",     Setting::TypeInt)     = (int) fDebug;
    MM.add("Logging",   Setting::TypeBoolean) = true;


    Setting &Geodetic = root.add("Geodetic", Setting::TypeGroup);
    Geodetic.add("StartingLat", Setting::TypeFloat) = fLatDegrees0;
    Geodetic.add("StartingLon", Setting::TypeFloat) = fLonDegrees0;

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
