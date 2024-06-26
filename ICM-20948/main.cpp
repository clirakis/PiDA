/**
 ******************************************************************
 *
 * Module Name : main.cpp 
 *
 * Author/Date : C.B. Lirakis / 05-Mar-19
 *
 * Description : IMU front end
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
// System includes.
#include <iostream>
using namespace std;
#include <cstring>
#include <cmath>
#include <csignal>
#include <unistd.h>
#include <time.h>
#include <fstream>
#include <cstdlib>

/// Local Includes.
#include "debug.h"
#include "tools.h"
#include "CLogger.hh"
#include "UserSignals.hh"
#include "Version.hh"
#include "IMU.hh"

/** Control the verbosity of the program output via the bits shown. */
static unsigned int VerboseLevel = 0;

/** Pointer to the logger structure. */
static CLogger   *logger;
static bool      selfTest = false;
static bool      magCal   = false;

/**
 ******************************************************************
 *
 * Function Name : Help
 *
 * Description : provides user with help if needed.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 *******************************************************************
 */
static void Help(void)
{
    SET_DEBUG_STACK;
    cout << "********************************************" << endl;
    cout << "* Test file for text Logging.              *" << endl;
    cout << "* Built on "<< __DATE__ << " " << __TIME__ << "*" << endl;
    cout << "* Available options are :                  *" << endl;
    cout << "*   -h Help                                *" << endl;
    cout << "*   -m magnetic calibration                *" << endl;
    cout << "*   -s Self test                           *" << endl;
    cout << "*   -v verbose level                       *" << endl;
    cout << "*                                          *" << endl;
    cout << "********************************************" << endl;
}
/**
 ******************************************************************
 *
 * Function Name :  ProcessCommandLineArgs
 *
 * Description : Loop over all command line arguments
 *               and parse them into useful data.
 *
 * Inputs : command line arguments. 
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static void
ProcessCommandLineArgs(int argc, char **argv)
{
    int option;
    SET_DEBUG_STACK;
    do
    {
        option = getopt( argc, argv, "hHmMsSv:");
        switch(option)
        {
        case 'h':
        case 'H':
            Help();
            Terminate(0);
	    break;
	case 'm':
	case 'M':
	    magCal = true;
	    break;
	case 's':
	case 'S':
	    selfTest = true;
	    break;
	case 'v':
	    if (optarg)
	    {
		VerboseLevel = atoi(optarg);
	    }
	    else
	    {
		VerboseLevel = 1;
	    }
            break;
        }
    } while(option != -1);
}
/**
 ******************************************************************
 *
 * Function Name : Initialize
 *
 * Description : Initialze the process
 *               - Setup traceback utility
 *               - Connect all signals to route through the terminate 
 *                 method
 *               - Perform any user initialization
 *
 * Inputs : none
 *
 * Returns : true on success. 
 *
 * Error Conditions : depends mostly on user code
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static bool Initialize(void)
{
    SET_DEBUG_STACK;
    char   msg[32];
    double version;

    SetSignals();
    // User initialization goes here. 
    sprintf(msg, "%d.%d",MAJOR_VERSION, MINOR_VERSION);
    version = atof( msg);
    logger = new CLogger("IMU.log", "IMU", version);
    logger->SetVerbose(VerboseLevel);

    return true;
}

/**
 ******************************************************************
 *
 * Function Name : main
 *
 * Description : It all starts here:
 *               - Process any command line arguments
 *               - Do any necessary initialization as a result of that
 *               - Do the operations
 *               - Terminate and cleanup
 *
 * Inputs : command line arguments
 *
 * Returns : exit code
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
int main(int argc, char **argv)
{
    ProcessCommandLineArgs(argc, argv);
    if (Initialize())
    {
	IMU *pModule = new IMU("IMU.cfg");

	if (!pModule->CheckError())
	{
	    if (selfTest)
	    {
		double Results[9];
		pModule->IMUSelfTest(Results);
		int16_t magval[3];
		pModule->MagSelfTest(magval);
		time_t now;
		char filename[256];
		time(&now);
		struct tm *tmnow = gmtime(&now);
		strftime( filename, sizeof(filename),"%y%m%d_%H%M%S.tst", 
			  tmnow);
		ofstream testrv(filename);
		testrv << "# ICM20948 and AK09916 selftest results performed on: "
		       << asctime(tmnow)
		       << "ACC:  " << Results[0] << "," << Results[1] << ","
		       << Results[2] << endl
		       << "GYRO: " << Results[3] << "," << Results[4] << ","
		       << Results[5] << endl
		       << "MAG:  " << magval[0] << "," << magval[1] << ","
		       << magval[2] << endl;
		testrv.close();


	    }
	    if (magCal)
	    {
		double bias[3], scale[3];
		time_t now;
		char filename[256];
		time(&now);
		struct tm *tmnow = gmtime(&now);
		strftime( filename, sizeof(filename),"%y%m%d_%H%M%S.cal", 
			  tmnow);
		pModule->MagCal(bias, scale); 
		ofstream cal(filename);
		cal << "# AK09916 calibration performed on: "
		    << asctime(tmnow)
		    << "# bias X,Y,Z and Scale X,Y,Z" << endl;
		cal << bias[0] << "," << bias[1] << "," << bias[2] << ","
		    << scale[0] << "," << scale[1] << "," << scale[2]
		    << endl;
		cal.close();
	    }
	    else
	    {
		pModule->Do();
	    }
	}

    }
    Terminate(0);
}
