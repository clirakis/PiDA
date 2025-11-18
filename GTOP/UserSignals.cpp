/********************************************************************
 *
 * Module Name : UserSignals.cpp
 *
 * Author/Date : C.B. Lirakis / 05-Mar-19
 *
 * Description : Terminate to be called from anywhere to do proper clean up
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * 08-Sep-25   CBL changed SIGUSR2 to force log filename change. 
 *
 * Classification : Unclassified
 *
 * References :
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <csignal>


// Local Includes.
#include "UserSignals.hh"
#include "debug.h"
#include "CLogger.hh"
#include "GTOP.hh"
#include "GTOPdisp.hh"

/**
 ******************************************************************
 *
 * Function Name : Terminate
 *
 * Description : Deal with errors in a clean way!
 *               ALL, and I mean ALL exits are brought 
 *               through here!
 * 
 * Inputs : Signal causing termination. 
 *
 * Returns : none
 *
 * Error Conditions : Well, we got an error to get here. 
 *
 *******************************************************************
 */ 
void Terminate (int sig) 
{
    static int i=0;
    CLogger *logger = CLogger::GetThis();
    char msg[128], tmp[64];
    time_t now;
    time(&now);
 
    i++;
    if (i>1) 
    {
        _exit(-1);
    }

    switch (sig)
    {
    case -1: 
      sprintf( msg, "User abnormal termination");
      break;
    case 0:                    // Normal termination
        snprintf( msg, sizeof(msg), "Normal program termination.");
        break;
    case SIGHUP:
        snprintf( msg, sizeof(msg), " Hangup");
        break;
    case SIGINT:               // CTRL+C signal 
        snprintf( msg, sizeof(msg), " SIGINT ");
        break;
    case SIGQUIT:               //QUIT 
        snprintf( msg, sizeof(msg), " SIGQUIT ");
        break;
    case SIGILL:               // Illegal instruction 
        snprintf( msg, sizeof(msg), " SIGILL ");
        break;
    case SIGABRT:              // Abnormal termination 
        snprintf( msg, sizeof(msg), " SIGABRT ");
        break;
    case SIGBUS:               //Bus Error! 
        snprintf( msg, sizeof(msg), " SIGBUS ");
        break;
    case SIGFPE:               // Floating-point error 
        snprintf( msg, sizeof(msg), " SIGFPE ");
        break;
    case SIGKILL:               // Kill!!!! 
        snprintf( msg, sizeof(msg), " SIGKILL");
        break;
    case SIGSEGV:              // Illegal storage access 
        snprintf( msg, sizeof(msg), " SIGSEGV ");
        break;
    case SIGTERM:              // Termination request 
        snprintf( msg, sizeof(msg), " SIGTERM ");
        break;
    case SIGTSTP:               // 
        snprintf( msg, sizeof(msg), " SIGTSTP");
        break;
    case SIGXCPU:               // 
        snprintf( msg, sizeof(msg), " SIGXCPU");
        break;
    case SIGXFSZ:               // 
        snprintf( msg, sizeof(msg), " SIGXFSZ");
        break;
    case SIGSTOP:               // 
        snprintf( msg, sizeof(msg), " SIGSTOP ");
        break;
    case SIGSYS:               // 
        snprintf( msg, sizeof(msg), " SIGSYS ");
        break;
#ifndef MAC
     case SIGPWR:               // 
        snprintf( msg, sizeof(msg), " SIGPWR ");
        break;
    case SIGSTKFLT:               // Stack fault
        snprintf( msg, sizeof(msg), " SIGSTKFLT ");
        break;
#endif
   default:
        snprintf( msg, sizeof(msg), " Uknown signal type: %d", sig);
        break;
    }
    if (sig!=0)
    {
        snprintf ( tmp, sizeof(tmp), " %s %d", LastFile, LastLine);
        strncat ( msg, tmp, sizeof(msg)-strlen(tmp));
	logger->LogCommentTimestamp(msg);
	//logger->Log("# %s\n",msg);
    }


    GTOP_Display *disp = GTOP_Display::GetThis();
    if(disp)
    {
	disp->Stop();
    }

    // User termination here
    GTOP *pGPS = GTOP::GetThis();
    delete pGPS;

    delete logger;

    if (sig == 0)
    {
        _exit (0);
    }
    else
    {
        _exit (-1);
    }
}
/**
 ******************************************************************
 *
 * Function Name : UserSignal
 *
 * Description : Alternative way to communicate with a program. 
 *
 * Inputs : sig - signal issued.
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
void UserSignal(int sig)
{
    CLogger *logger = CLogger::GetThis();
    logger->Log("# SIGUSR: %d\n", sig);

    switch (sig)
    {
    case SIGUSR1:   // 10
	/* 
	 * man pages have this as possibly 30,10 or 16, depends
	 * on OS
	 */
	/* Command a graceful exit to the program. */
	GTOP::GetThis()->Stop(); 
	break;
    case SIGUSR2:   // 12
	/*! 
	 * Man pages have this listed as 31,12, or 17
	 * Ask the program to change filenames. 
	 */
	logger->Log("# user request filename change.\n");
	GTOP::GetThis()->UpdateFileName();
	break;
    }
}
/**
 ******************************************************************
 *
 * Function Name : SetSignals
 *
 * Description : Route termination signals through exit method. 
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : None
 * 
 * Unit Tested on: 23-Feb-08
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void SetSignals(void)
{
    /*
     * Setup a signal handler.      
     */
    signal (SIGHUP , Terminate);   // Hangup.
    signal (SIGINT , Terminate);   // CTRL+C signal 
    signal (SIGKILL, Terminate);   // 
    signal (SIGQUIT, Terminate);   // 
    signal (SIGILL , Terminate);   // Illegal instruction 
    signal (SIGABRT, Terminate);   // Abnormal termination 
    signal (SIGIOT , Terminate);   // 
    signal (SIGBUS , Terminate);   // 
    signal (SIGFPE , Terminate);   // 
    signal (SIGSEGV, Terminate);   // Illegal storage access 
    signal (SIGTERM, Terminate);   // Termination request 
    signal (SIGSTOP, Terminate);   // 
    signal (SIGSYS, Terminate);    // 
#ifndef MAC
    signal (SIGSTKFLT, Terminate); // 
    signal (SIGPWR, Terminate);    // 
#endif
    // Setup user signals for further control
    signal (SIGUSR1, UserSignal);
    signal (SIGUSR2, UserSignal);  

}
