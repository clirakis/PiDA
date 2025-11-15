/**
 ******************************************************************
 *
 * Module Name : tsip_utilities.c
 *
 * Author/Date : C.B. Lirakis / 25-Dec-15
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *******************************************************************
 */
/* System includes. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <time.h>

/** Local Includes. */
#include "debug.h"
#include "GTOP_utilities.h"

/** Global Variables. */

/**
 ******************************************************************
 *
 * Function Name : DateFromGPSTime
 *
 * Description : Conversion from internal time standards to something
 * that is usable. 
 *       TimeOfWeek runs 0-1023 
 *          Week zero was January 6, 1980. This was crossed August 22, 1999.
 *       Extended GPS week number, August 22, 1999 is wek 1024. 
 *
 *          
 *
 * Inputs : TimeOfWeek    - time since our epoch. 
 *          Extended Week - see above
 *          UTC_Offset    - seconds from GPS time that UTC is 
 *
 * Returns : timespec of GPSWeek. 
 *
 * Error Conditions :
 *
 *******************************************************************
 */
struct timespec DateFromGPSTime(float TimeOfWeek, int ExtendedWeek, float UTC_Offset)
{
    const unsigned  SecPerDay  = 60*60*24;
    const unsigned  SecPerWeek = SecPerDay*7;
    // First Epoch is January 6, 1980. 
    //const time_t    gps_epoch1 = 315964800;
    // Second Epoch is August 22, 1999
    const time_t    gps_epoch2 = 935280000;
    struct timespec rv;

    double fsec = (double)(gps_epoch2 + ExtendedWeek*SecPerWeek);
    fsec       -= UTC_Offset;
    fsec       += TimeOfWeek;

    rv.tv_sec  = (unsigned long) floor(fsec);
    rv.tv_nsec = (unsigned long) floor(((double)(fsec-rv.tv_sec))*1.0e9);

    return rv;
}

void ClearBit(unsigned char *mybyte,unsigned char mask)
{
    *mybyte &= ~mask;
}

void SetBit(unsigned char *mybyte,unsigned char mask)
{
    *mybyte |= mask;
}

void ToggleBit(unsigned char *mybyte,unsigned char mask)
{
    if ((*mybyte&mask) > 0)
    {
	ClearBit( mybyte, mask);
    }
    else
    {
	SetBit( mybyte, mask);
    }
}
