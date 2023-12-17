/********************************************************************
 *
 * Module Name : IMUData.cpp
 *
 * Author/Date : C.B. Lirakis / 27-Feb-22
 *
 * Description : Generic module
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * 17-Dec-23 Added in Lat/Lon data
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

// Local Includes.
#include "debug.h"
#include "IMUData.hh"

IMUData* IMUData::fIMUData;

/**
 ******************************************************************
 *
 * Function Name : IMUData constructor
 *
 * Description : Zero all the data inputs
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
IMUData::IMUData(void)
{
   SET_DEBUG_STACK;
   fIMUData = this;
    /*
     * Zero all input vectors
     */
    memset( fAcc,  0, 3*sizeof(double));
    memset( fMag,  0, 3*sizeof(double));
    memset( fGyro, 0, 3*sizeof(double));
    fTemp    = 0.0;
    fMagRead = false;
}

/**
 ******************************************************************
 *
 * Function Name : module destructor
 *
 * Description : PLACEHOLDER
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
IMUData::~IMUData (void)
{
}
/**
 ******************************************************************
 *
 * Function Name :  <<
 *
 * Description : operator to print out values from last read
 *      switches output based on what sensors are operating. 
 *
 * Inputs : 
 *     output - ostream to place data into
 *     n - IMUData class. 
 *
 * Returns : populated ostream
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 27-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
ostream& operator<<(ostream& output, const IMUData &n)
{
    SET_DEBUG_STACK;
    //char fmttime[128];
    //struct tm *tme = localtime(&n.fReadTime.tv_sec);
    //strftime( fmttime, sizeof(fmttime), " ", &tme);
    double fracsec = n.fReadTime.tv_nsec;
    fracsec = fracsec*1.0e-9;
    output << "IMUData " << ctime(&n.fReadTime.tv_sec) 
	   << "    Frac: " << fracsec << endl;
    
    output << "    Temperature: " << n.fTemp << " C" << endl
	   << "    Acceleration " 
	   << " X: " << n.fAcc[0] 
	   << " Y: " << n.fAcc[1]
	   << " Z: " << n.fAcc[2] << endl
	   << "             Gyro" 
	   << " X: " << n.fGyro[0] 
	   << " Y: " << n.fGyro[1]
	   << " Z: " << n.fGyro[2] << endl;

    if (n.fMagRead)
    {
	output << "         Magnetic"  
	       << " X: " << n.fMag[0] 
	       << " Y: " << n.fMag[1]
	       << " Z: " << n.fMag[2] << endl;
    }
    else
    {
	output << "Magnetic read timed out.\n" << endl;
    }
    SET_DEBUG_STACK;
    return output;
}
