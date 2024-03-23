/**
 ******************************************************************
 *
 * Module Name : Timing.cpp
 *
 * Author/Date : C.B. Lirakis / 17-Mar-24
 *
 * Description : Timing main file. 
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : 
 *              https://blog.meinbergglobal.com/2021/02/25/the-root-of-all-timing-understanding-root-delay-and-root-dispersion-in-ntp/
 *
 * https://www.timelinkmicro.info/ntp-basic-how-it-works/
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
#include "Timing.hh"
#include "queryTimeServer.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"

Timing* Timing::fTiming;

static uint32_t NVar = 10;

/**
 ******************************************************************
 *
 * Function Name : Timing constructor
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
Timing::Timing(const char* ConfigFile) : CObject()
{
    CLogger *Logger = CLogger::GetThis();

    /* Store the this pointer. */
    fTiming = this;
    SetName("Timing");
    SetError(); // No error.

    fRun = true;
    fQS  = NULL;
    fNSamples = 1;
    fSampleRate = 1;

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
	fn = new FileName("Timing", "h5", One_Day);
	OpenLogFile();
    }

    Logger->Log("# Timing constructed.\n");

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Timing Destructor
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
Timing::~Timing(void)
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
    delete f5Logger;
    f5Logger = NULL;

    delete fQS;

    // Make sure all file streams are closed
    Logger->Log("# Timing closed.\n");
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
void Timing::Do(void)
{
    SET_DEBUG_STACK;
    CLogger *pLogger = CLogger::GetThis();
    struct timespec  host_now;    
    struct timespec  value;
    struct tm        *tme;
    time_t           sec; 
    double           delta;
    double           PCsec; 
    const struct pkt *pMSG;
    double           prec, delay, disp, tod;
    double           dResponse = 0.0;
    double           dTotal;
    double           dsec;
    int32_t          count = 0;
    double           multiplier = pow(2.0, -32);


    fRun = true;

    while(fRun)
    {
	value = fQS->GetTime();
	prec  = fQS->Precision();
	delay = fQS->Drootdelay();
	disp  = fQS->Drootdispersion();
	//sec   = value.tv_sec;
	sec = time(NULL);
	tme   = localtime(&sec);
	//sec   -= tme->tm_gmtoff;
	tod   = tme->tm_sec + 60*(tme->tm_min + 60*tme->tm_hour); 
	PCsec = (double)sec + 1.0e-9 * ((double) host_now.tv_nsec);

	clock_gettime(CLOCK_REALTIME, &host_now);   // Host time
	delta = (double)(host_now.tv_sec - value.tv_sec) + 
	    1.0e-9 * (double)(host_now.tv_nsec - value.tv_nsec);

	pMSG = fQS->GetMSG();

	/* 
	 * Algorithm is as follows
	 * Definitions
	 *     T1 originator time stamp
	 *     T2 request received at server
	 *     T3 Transmit response from server
	 *     T4 time received at client. 
	 * 
	 *     delays = (T4-T1) - (T3-T2); 
	 *     
	 */

	f5Logger->FillInternalVector(PCsec, 0);
	f5Logger->FillInternalVector(tod,   1);
	f5Logger->FillInternalVector(prec,  2);
	f5Logger->FillInternalVector(delay, 3);
	f5Logger->FillInternalVector(disp,  4);
	f5Logger->FillInternalVector(delta, 5);
	dsec = (double) pMSG->rec[0] + 
	    multiplier * ((double) pMSG->rec[1]);
	f5Logger->FillInternalVector(dsec, 6);

	dsec = (double) pMSG->xmt[0] + 
	    multiplier * ((double) pMSG->xmt[1]);
	f5Logger->FillInternalVector(dsec, 7);
	f5Logger->FillInternalVector(dResponse, 8);
	dTotal = fQS->Correction();
	f5Logger->FillInternalVector(dTotal, 9);

	f5Logger->Fill();

	count++;
	if (count%100) 
	    pLogger->LogTime("Samples processed: %d of %d\n", 
			     count, fNSamples);
	if (fRun)  // User didn't request a stop.
	    fRun = (count<fNSamples);
	sleep(fSampleRate);
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
bool Timing::OpenLogFile(void)
{
    SET_DEBUG_STACK;

    // USER TO FILL IN.
    const char *Names   = "PCTime:TOD:PREC:DELAY:DISP:DELTA:REC:XMIT:DRESPONSE:DTOTAL";
    CLogger    *pLogger = CLogger::GetThis();
    /* Give me a file name.  */
    const char* name = fn->GetUniqueName();
    fn->NewUpdateTime();
    SET_DEBUG_STACK;

    f5Logger = new H5Logger(name,"NTP Logger Dataset", NVar, false);
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
bool Timing::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *pLogger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();
    const char *ServerAddress;
    int    Debug = 0;

    /*
     * Open the configuragtion file. 
     */
    try{
	pCFG->readFile(fConfigFileName);
    }
    catch( const FileIOException &fioex)
    {
	pLogger->LogError(__FILE__,__LINE__,'F',
			 "I/O error while reading configuration file.\n");
	return false;
    }
    catch (const ParseException &pex)
    {
	pLogger->Log("# Parse error at: %s : %d - %s\n",
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
	 * index into group Timing
	 */
	const Setting &MM = root["Timing"];
	MM.lookupValue("Logging",   fLogging);
	MM.lookupValue("Debug",     Debug);
	MM.lookupValue("Server",    ServerAddress);
	MM.lookupValue("Samples",   fNSamples);
	MM.lookupValue("SampleRate",fSampleRate);
	SetDebug(Debug);
    }
    catch(const SettingNotFoundException &nfex)
    {
	// Ignore.
    }
    if (fNSamples>0) 
    {
	time_t now;
	time(&now);
	now += (fNSamples * fSampleRate);
	struct tm *tnow = gmtime(&now);
	pLogger->LogTime("NSamples %d, Sample Rate: %d finish at: %s\n", 
			 fNSamples, fSampleRate, asctime(tnow));

    }
    // If life is good create the queryServer 
    pLogger->LogTime("Trying timeserver: %s\n", ServerAddress);
    fQS = new QueryTS(ServerAddress, (Debug>0));
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
bool Timing::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    Setting &root = pCFG->getRoot();

    // USER TO FILL IN
    // Add some settings to the configuration.
    Setting &MM = root.add("Timing", Setting::TypeGroup);
    MM.add("Debug",       Setting::TypeInt)     = 0;
    MM.add("Logging",     Setting::TypeBoolean) = true;
    if (fQS)
    {
	MM.add("Server",  Setting::TypeString)  = fQS->GetServerName();
    }
    else
    {
	MM.add("Server" , Setting::TypeString)  = "time.nist.gov";
    }
    MM.add("Samples",     Setting::TypeInt)     = fNSamples;
    MM.add("SampleRate",  Setting::TypeInt)     = fSampleRate;

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
