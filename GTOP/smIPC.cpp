/********************************************************************
 *
 * Module Name : smIPC.cpp
 *
 * Author/Date : C.B. Lirakis / 20-Feb-22
 *
 * Description : Shared memory IPC for NMEA GPS data
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
 * 21-Sep-20    CBL    Changed over to SharedMem2
 * 06-Feb-22    CBL    Modified to add the current data file name 
 *                     into shared memory. '
 * 20-Feb-22    CBL    NMEA GPS
 * 15-Dec-23    CBL    Put in some logging to show that SM was connected
 *
 * Classification : Unclassified
 *
 * References : NONE
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <cstring>

// Local Includes.
#include "debug.h"
#include "CLogger.hh"
#include "SharedMem2.hh"     // class definition for shared segment. 
#include "smIPC.hh"
#include "GTOP.hh"

const size_t kCommandSize  = 32;          // Bytes of command data.
const size_t kFilenameSize = 512;         // Bytes of data open to filename
#define DEBUG_SM 0
/**
 ******************************************************************
 *
 * Function Name : IPC_Initialize
 *
 * Description : Create all necessary shared memory segments
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 23-Aug-14
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
GPS_IPC::GPS_IPC(void) : CObject()
{
    SET_DEBUG_STACK;

    //NMEA_GPS* gps = NMEA_GPS::GetThis();
    CLogger *plogger = CLogger::GetThis();

    SetName("GPS_IPC");
    SetError(); // No error.
    SetDebug(0);

    plogger->LogCommentTimestamp("IPC Initialize, shared memory.");

    pSM_Commands      = NULL;
    pSM_PositionData  = NULL;
    pSM_SolutionData  = NULL;
    pSM_VelocityData  = NULL;
    pSM_Minimum       = NULL;
    fSM_Filename      = NULL;

    pSM_PositionData = new SharedMem2("GGA", 
				      GGA::DataSize(), true);
    if (pSM_PositionData->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "GGA data SM failed.");
	delete pSM_PositionData;
	pSM_PositionData = 0;
	SET_DEBUG_STACK;
	SetError(-1);
    }
    else
    {
	plogger->Log("# GGA SM successfully created.\n");
    }


#if 0
    cout << "TESTME: " << s->DataSize() 
	 << " And sizeof: " << sizeof(SolutionStatus) << " "
	 << SolutionStatus::DataSize()
	 << endl;
#endif
    pSM_SolutionData = new SharedMem2("GSA", 
				      GSA::DataSize(),true);
    if (pSM_SolutionData->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "GSA data SM failed.");
	delete pSM_SolutionData;
	pSM_SolutionData = NULL;
	SET_DEBUG_STACK;
	SetError(-2);
    }
    else
    {
	plogger->Log("# GSA status SM successfully created.\n");
    }

    pSM_VelocityData = new SharedMem2("VTG", 
				      VTG::DataSize(), true);
    if (pSM_VelocityData->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "VTG data SM failed.");
	delete pSM_VelocityData;
	pSM_VelocityData = NULL;
	SET_DEBUG_STACK;
	SetError(-3);
    }
    else
    {
	plogger->Log("# VTG SM successfully created.\n");
    }

    pSM_Minimum = new SharedMem2("RMC", 
				      RMC::DataSize(), true);
    if (pSM_VelocityData->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "RMC data SM failed.");
	delete pSM_Minimum;
	pSM_Minimum = NULL;
	SET_DEBUG_STACK;
	SetError(-4);
    }
    else
    {
	plogger->Log("# VTG SM successfully created.\n");
    }

#if 0
    pSM_Commands = new SharedMem2("GPS_Commands", kCommandSize, true);
    if (pSM_Commands->CheckError())
    {
	plogger->Log("# %s %d Commands shared memory failed.\n", 
		    __FILE__,  __LINE__);
	delete pSM_Commands;
	pSM_Commands = NULL;
	SetError(-5);
    }
    else
    {
	plogger->Log("# %s %d Commands shared memory attached.\n",
		    __FILE__,  __LINE__);
    }
#endif
    fSM_Filename = new SharedMem2("GPS_Filename", kFilenameSize, true);
    if (fSM_Filename->CheckError())
    {
	plogger->Log("# %s %d Filename shared memory failed.\n", 
		    __FILE__,  __LINE__);
	delete fSM_Filename;
	fSM_Filename = NULL;
	SetError(-5);
    }
    else
    {
	plogger->Log("# %s %d Filename shared memory attached: GPS_Filename\n",
		     __FILE__,  __LINE__);
    }

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  ProcessCommands
 *
 * Description : Process any commmands that have been issued by
 * the parent process. 
 *
 * Inputs : gps - lassen gps structure.
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
void GPS_IPC::ProcessCommands(void)
{
    SET_DEBUG_STACK;
    CLogger *plogger = CLogger::GetThis();
    if (pSM_Commands != NULL)
    {
        // number of bytes in buffer
	float available = pSM_Commands->GetData(); 
	char command[kCommandSize];
	if (available > 0.0)
	{
	    pSM_Commands->GetData(command);
	    plogger->Log("# Command received: %s\n", command);
#if 0
	    // Process approprately.
	    if (strcmp( command, "CF") == 0)
	    {
		puser->ChangeFile();
	    }
#endif
	    available = 0.0; // Commands have been processed. 
	    memset( command, 0, kCommandSize);
	    pSM_Commands->PutData(available);
	}
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  Update
 *
 * Description : Update all gps data in shared memory.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 21-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void GPS_IPC::Update(void)
{
    SET_DEBUG_STACK;
    GTOP *pGTOP    = GTOP::GetThis();
    NMEA_GPS* pGPS = pGTOP->GetNMEAGPS();

    if (pGPS)
    {
	GGA* pGGA = pGPS->pGGA();   // Full position
	VTG* pVTG = pGPS->pVTG();   // course and speed
	GSA* pGSA = pGPS->pGSA();   // Active satellites
	RMC* pRMC = pGPS->pRMC();   // Postion and CMG etc

	SET_DEBUG_STACK;
	if (pGGA && pSM_PositionData)
	{
	    // DEBUG!!
	    double tmp = pGGA->HDOP();
	    pSM_PositionData->PutData(tmp);
	    pSM_PositionData->PutData(pGGA->DataPointer());
	}

	SET_DEBUG_STACK;
	if(pGSA && pSM_SolutionData)
	{
	    pSM_SolutionData->PutData(pGSA->DataPointer());
	}

	SET_DEBUG_STACK;
	if (pVTG && pSM_VelocityData)
	{
	    pSM_VelocityData->PutData(pVTG->DataPointer());
	}

	
	if (pRMC && pSM_Minimum)
	{
	    pSM_Minimum->PutData( pRMC->DataPointer());
	}

	ProcessCommands();
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  UpdateFilename
 *
 * Description : Update the current data file name, includes path. 
 *
 * Inputs : full file specification to the current data file. 
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
void GPS_IPC::UpdateFilename(const char *name)
{
    SET_DEBUG_STACK;
    char temp[kFilenameSize];


    if (fSM_Filename)
    {
	memset( temp, 0, kFilenameSize);
	strncpy( temp, name, kFilenameSize-1);
	fSM_Filename->PutData(temp);
    }

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  GPS_IPC destructor
 *
 * Description : clean up any allocated data 
 *
 * Inputs : NONE
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
GPS_IPC::~GPS_IPC(void)
{
    SET_DEBUG_STACK;
    delete pSM_PositionData;
    delete pSM_SolutionData;
    delete pSM_VelocityData;
    delete pSM_Minimum;
    delete pSM_Commands;
    delete fSM_Filename;
    SET_DEBUG_STACK;
}


