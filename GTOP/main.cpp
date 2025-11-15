/**
 ******************************************************************
 *
 * Module Name : main.cpp
 *
 * Author/Date : C.B. Lirakis / 24-Dec-15
 *
 * Description : My GTOP interface
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * 18-Feb-22  CBL   Allow the display to be turned off. 
 *                  set the startup characteristics in a cfg file
 *
 * Classification : Unclassified
 *
 * References :
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


/** Local Includes. */
#include "debug.h"
#include "GTOPdisp.hh"
#include "GTOP.hh"
#include "Version.hh"
#include "CLogger.hh"
#include "UserSignals.hh"

/** Global Variables. **********************************************/

/** 
 * Pointer to the logger structure. 
 * This is for error messages and tells the user about all the issues etc. 
 */
static CLogger   *logger;

/**
 * thread control for the display if selected. 
 */
static pthread_t d_thread;

/** 
 * pointer to display. Normally we won't use this, but during bringup and
 * debug phases it is extremly useful. 
 */
static GTOP_Display *pDisp;

/** 
 * Specify a default config file name. 
 * The user may override this at the command line using the -c command. 
 */
static const char*   ConfigFileName = "gtop.cfg";

/**
 * By default the user display is off. 
 */
static bool DisplayON = false;

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
    /*    SET_DEBUG_STACK; */
    printf("********************************************\n");
    printf("* GTop interface                           *\n");
    printf("* Built on %s %s *\n", __DATE__ , __TIME__  );
    printf("* Available options are :                  *\n");
    printf("* -c Configuration File Name               *\n");
    printf("* -h this help text                        *\n");
    printf("* -d interactive screen display.           *\n");
    printf("********************************************\n");
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
        option = getopt( argc, argv, "c:C:dDhH");
        switch(option)
	{
	case 'c':
	case 'C':
	    ConfigFileName = strdup(optarg);
	    break;
	case 'd':
	case 'D':
	    DisplayON = true;
	    break;
	case 'h':
	case 'H':
	    Help();
	    Terminate(0);
	    break;
	}
    } while(option != -1);

    SET_DEBUG_STACK;
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
    LastFile = (char *) __FILE__;
    LastLine = __LINE__;
    char   msg[32];
    double version;

    SetSignals();
    // User initialization goes here. 
    /* startup a message log. String messages go here. */
    sprintf(msg, "%d.%d",MAJOR_VERSION, MINOR_VERSION);
    version = atof( msg);

    /*
     * Startup an error logger.
     */
    logger = new CLogger("gtop.log", "gtop", version);
    //logger->SetVerbose(VerboseLevel);

    /*
     * If user has specified a display, set it up now. 
     */
    if (DisplayON)
    {
	/* If the user has requested the display feature, start it now. */
	/* create the display. */
	pDisp = new GTOP_Display();
	if( pthread_create(&d_thread, NULL, DisplayThread, NULL) == 0)
	{
	    logger->Log("# Display Thread successfully created.\n");
	}
	else
	{
	    SET_DEBUG_STACK;
	    /* It is not the end of the world if this fails. */
	    logger->Log("# Dispaly Thread failed.\n");
	}
    }
    logger->Log("# SIGUSR1: %d, SIGUSR2 %d\n", SIGUSR1, SIGUSR2);
    return true;
}
/**
 ******************************************************************
 *
 * Function Name :
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
int main(int argc, char **argv)
{
    ProcessCommandLineArgs( argc, argv);
    if (Initialize())
    {
	GTOP *pGPS = new GTOP("gtop.cfg");

	if (pGPS->Error() == 0)
	{
	    pGPS->Do();
	}
    }
    Terminate(0);
    exit(0);
}

