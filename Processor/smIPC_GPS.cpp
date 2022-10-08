/********************************************************************
 *
 * Module Name : smIPC_GPS.cpp
 *
 * Author/Date : C.B. Lirakis / 23-Feb-22
 *
 * Description : Shared memory read for IPC for NMEA GPS data
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
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
#include "smIPC_GPS.hh"

const size_t kCommandSize = 32;           // Bytes of command data.
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

    CLogger *plogger = CLogger::GetThis();

    SetName("GPS_IPC");
    SetError(); // No error.
    SetDebug(0);

    plogger->LogCommentTimestamp("GPS IPC Initialize, shared memory.");

    pSM_PositionData  = NULL;
    pSM_SolutionData  = NULL;
    pSM_VelocityData  = NULL;
    pSM_Minimum       = NULL;

    fRMC = NULL;
    fGGA = NULL;
    fVTG = NULL;
    fGSA = NULL;

    pSM_PositionData = new SharedMem2("GGA"); 
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
	plogger->Log("# GGA SM successfully attached.\n");
    }

    pSM_SolutionData = new SharedMem2("GSA");
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
	plogger->Log("# GSA status SM successfully attached.\n");
    }

    pSM_VelocityData = new SharedMem2("VTG");
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
	plogger->Log("# Velocity SM successfully attached.\n");
    }


    /*
     * This is the data containing the GPS delta information.
     * Open shared memory to share with the world our data. 
     * Put the entire GPSTime structure out there
     */

    pSM_Minimum = new  SharedMem2("RMC");
    if (pSM_Minimum->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W', 
			  "RMCshared memory FAIL.");
	delete pSM_Minimum;
	pSM_Minimum = NULL;
	SetError(-1, __LINE__);
	SET_DEBUG_STACK;
	return;
    }

    fRMC = new RMC();
    fGGA = new GGA();
    fVTG = new VTG();
    fGSA = new GSA();

    SET_DEBUG_STACK;
}


/**
 ******************************************************************
 *
 * Function Name :  Update
 *
 * Description : 
 *       Update all gps data in shared memory.
 *       We can easily turn off checking of any of the shared memory
 *       items by setting any of the pointers to NULL. Assuming they 
 *       are not null, then we check if the LAM is set to true. 
 *
 *       If LAM is true, then the sender has populated with new data. 
 *       Alternatively you can check the date/time of the shared memory
 *       update. The user is responsible for Clearing the LAM saying
 *       that the data has been retrieved. 
 *
 *       For this particular implementation, any one of the LAMs being
 *       set indicates that the entire set is valid. This is based
 *       on the coding of the sender but is not in general true. 
 *
 * Inputs : none
 *
 * Returns : true when the data is new. 
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
bool GPS_IPC::Update(void)
{
    SET_DEBUG_STACK;

    /* Assume no new data. */
    bool rv = false;

    /*
     * Data is updated all at once if data is new for one it is new for
     * all. 
     * Check to see if the SM placement is new???
     */

    if (fGGA && pSM_PositionData)
    {
	/* Is the data "new" */
	if (pSM_PositionData->GetLAM())
	{
	    pSM_PositionData->GetData(fGGA->DataPointer());
	    pSM_PositionData->ClearLAM();
	    rv = true;
	}
    }

    SET_DEBUG_STACK;
    if(fGSA && pSM_SolutionData)
    {

	/* Is the data "new" */
	if (pSM_SolutionData->GetLAM())
	{
	    pSM_SolutionData->GetData(fGSA->DataPointer());
	    pSM_SolutionData->ClearLAM();
	    rv = true;
	}
    }

    SET_DEBUG_STACK;
    if (fVTG && pSM_VelocityData)
    {
	/* Is the data "new" */
	if (pSM_VelocityData->GetLAM())
	{
	    pSM_VelocityData->GetData(fVTG->DataPointer());
	    pSM_VelocityData->ClearLAM();
	    rv = true;
	}
    }
	
    if (fRMC && pSM_Minimum)
    {
	/* Is the data "new" */
	if (pSM_Minimum->GetLAM())
	{
	    pSM_Minimum->GetData(fRMC->DataPointer());
	    pSM_Minimum->ClearLAM();
	    rv = true;
	}
    }

    SET_DEBUG_STACK;
    return rv;
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

    delete fRMC;
    delete fGGA;
    delete fVTG;
    delete fGSA;

    delete pSM_PositionData;
    delete pSM_SolutionData;
    delete pSM_VelocityData;
    delete pSM_Minimum;

    SET_DEBUG_STACK;
}


