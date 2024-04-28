/********************************************************************
 *
 * Module Name : smIPC.cpp
 *
 * Author/Date : C.B. Lirakis / 24-Apr-24
 *
 * Description : Shared memory IPC for BARO data
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
#include "SharedMem2.hh"     // class definition for shared segment. 
#include "smIPC.hh"


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
BARO_IPC::BARO_IPC(void) : CObject()
{
    SET_DEBUG_STACK;
    Barometer *ptr   = Barometer::GetThis();
    CLogger   *plogger = CLogger::GetThis();

    SetName("BARO_IPC");
    SetError(); // No error.
    SetDebug(0);

    plogger->LogCommentTimestamp("IPC Initialize, shared memory.");

    pSM          = NULL;
    pSM_Position = NULL;
    fSM_Filename = NULL;
    fGGA         = NULL;

    pSM = new SharedMem2("BARO", ptr->DataSize(), true);
    if (pSM->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W', "Baro data SM failed.");
	delete pSM;
	pSM = NULL;
	SET_DEBUG_STACK;
	SetError(-1);
    }
    else
    {
	plogger->Log("# BARO SM successfully created. %d\n", ptr->DataSize());
    }


    // Connect to GGA message if available. 
    pSM_Position = new SharedMem2("GGA"); 
    if (pSM_Position->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "GGA data SM failed.");
	delete pSM_Position;
	pSM_Position = 0;
	SET_DEBUG_STACK;
	SetError(-1);
    }
    else
    {
	plogger->Log("# GGA SM successfully attached.\n");
    }
    fGGA = new GGA();

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
void BARO_IPC::Update(void)
{
    SET_DEBUG_STACK;
    Barometer *ptr = Barometer::GetThis();
    if (ptr)
    {
	pSM->PutData(ptr->DataPointer());
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  GetPosition
 *
 * Description : Get the GGA message if available. 
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
GGA* BARO_IPC::GetPosition(void) const
{
    SET_DEBUG_STACK;
    if (fGGA && pSM_Position)
    {
	/* Is the data "new" */
	if (pSM_Position->GetLAM())
	{
	    pSM_Position->GetData(fGGA->DataPointer());
	    pSM_Position->ClearLAM();
	}
    }
    SET_DEBUG_STACK;
    return fGGA;
}


/**
 ******************************************************************
 *
 * Function Name :  BARO_IPC destructor
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
BARO_IPC::~BARO_IPC(void)
{
    SET_DEBUG_STACK;
    delete pSM;
    delete pSM_Position;
    delete fGGA;
    SET_DEBUG_STACK;
}


