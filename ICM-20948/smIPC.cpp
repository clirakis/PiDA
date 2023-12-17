/********************************************************************
 *
 * Module Name : smIPC.cpp
 *
 * Author/Date : C.B. Lirakis / 27-Feb-22
 *
 * Description : Shared memory IPC for IMU data
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
#include "IMUData.hh"


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

    plogger->LogCommentTimestamp("IPC Initialize, shared memory.");

    pSM          = NULL;
    pSM_Position = NULL;
    fSM_Filename = NULL;
    fGGA         = NULL;

    pSM = new SharedMem2("IMU", IMUData::DataSize(), true);
    if (pSM->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "IMU data SM failed.");
	delete pSM;
	pSM = NULL;
	SET_DEBUG_STACK;
	SetError(-1);
    }
    else
    {
	plogger->Log("# IMU SM successfully created. %d\n", IMUData::DataSize());
    }


    fSM_Filename = new SharedMem2("IMU_Filename", kFilenameSize, true);
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
void IMU_IPC::Update(void)
{
    SET_DEBUG_STACK;
    IMUData *ptr = IMUData::GetThis();
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
GGA* IMU_IPC::GetPosition(void) const
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
void IMU_IPC::UpdateFilename(const char *name)
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
    delete pSM_Position;
    delete fGGA;
    SET_DEBUG_STACK;
}


