/********************************************************************
 *
 * Module Name : EventCounter.cpp
 *
 * Author/Date : C.B. Lirakis / 23-May-21
 *
 * Description : Generic EventCounter
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
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

// Local Includes.
#include "debug.h"
#include "EventCounter.hh"

/**
 ******************************************************************
 *
 * Function Name : EventCounter constructor
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
EventCounter::EventCounter (bool Server) : SharedMem2("EventCounter",
						      sizeof(uint32_t),
						      Server, 0)
{
    SET_DEBUG_STACK;
    fCount  = 0;
    fServer = Server;
}

/**
 ******************************************************************
 *
 * Function Name : EventCounter destructor
 *
 * Description : NOTHING TO BE DONE HERE
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 15-Nov-25
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
EventCounter::~EventCounter(void)
{
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Count
 *
 * Description : Return Count, for server simple. For client
 *               do a read. 
 *
 * Inputs : none
 *
 * Returns : current count. 
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
uint32_t EventCounter::Count(void)
{
    SET_DEBUG_STACK;
    uint32_t rc = 0;

    if (fServer)
    {
	rc = fCount;
    }
    else
    {
	if (GetLAM())
	{
	    GetData(&fCount);
	    ClearLAM();
	}
    }
    SET_DEBUG_STACK;
    return rc;
}
/**
 ******************************************************************
 *
 * Function Name : Increment
 *
 * Description : IF we are the server 
 *                   - increment the count
 *                   - write the new count to SM
 *                   - Return Count AFTER increment
 *
 *               IF we are the client just return the count
 *
 * Inputs : none
 *
 * Returns : current count. 
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
uint32_t EventCounter::Increment(void)
{
    SET_DEBUG_STACK;
    if (fServer)
    {
	fCount++;
	PutData(&fCount);
    }
    SET_DEBUG_STACK;
    return fCount;
}

