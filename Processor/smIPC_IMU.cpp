/********************************************************************
 *
 * Module Name : smIPC_IMU.cpp
 *
 * Author/Date : C.B. Lirakis / 27-Feb-22
 *
 * Description : Shared memory IPC for IMU data, read side
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
#include "smIPC_IMU.hh"

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
IMU_IPC::IMU_IPC(void) : CObject()
{
    SET_DEBUG_STACK;

    CLogger *plogger = CLogger::GetThis();

    SetName("IMU_IPC");
    SetError(); // No error.
    SetDebug(0);

    plogger->LogCommentTimestamp("IPC IMU client Initialize, shared memory.");

    pSM          = NULL;
    fSM_Filename = NULL;
    fIMU         = NULL;

    pSM = new SharedMem2("IMU");
    if (pSM->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',"IMU data SM failed.");
	delete pSM;
	pSM = NULL;
	SET_DEBUG_STACK;
	SetError(-1);
    }
    else
    {
	plogger->Log("# IMU SM successfully created.\n");
    }


    fSM_Filename = new SharedMem2("IMU_Filename");
    if (fSM_Filename->CheckError())
    {
	plogger->Log("# %s %d Filename shared memory failed.\n", 
		    __FILE__,  __LINE__);
	SetError(-1);
    }
    else
    {
	plogger->Log("# %s %d Filename shared memory attached: IMU_Filename\n",
		     __FILE__,  __LINE__);
    }

    fIMU = new IMUData();

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
bool IMU_IPC::Update(void)
{
    SET_DEBUG_STACK;

    /* Assume no new data. */
    bool rv = false;
    /*
     * Data is updated all at once if data is new for one it is new for
     * all. 
     * Check to see if the SM placement is new???
     */

    if (fIMU && pSM)
    {
	/* Is the data "new" */
	if (pSM->GetLAM())
	{
	    pSM->GetData(fIMU->DataPointer());
	    pSM->ClearLAM();
	    rv = true;
	}
    }
    SET_DEBUG_STACK;
    return rv;
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
void IMU_IPC::UpdateFilename(void)
{
    SET_DEBUG_STACK;
    char temp[kFilenameSize];
#if 0
    if (fSM_Filename)
    {
	memset( temp, 0, kFilenameSize);
	strncpy( temp, name, kFilenameSize-1);
	fSM_Filename->PutData(temp);
    }
#endif
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  IMU_IPC destructor
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
IMU_IPC::~IMU_IPC(void)
{
    SET_DEBUG_STACK;
    delete pSM;
    delete fSM_Filename;
    SET_DEBUG_STACK;
}


