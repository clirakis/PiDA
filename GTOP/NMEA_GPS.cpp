/***********************************
This is our GPS library

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
*
* CBL Modified to run on PI. Also, I want to read the sentances
* elsewhere and pass them in to be parsed. 
*
****************************************/
#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

// Local includes
#include "tools.h"        // Units conversions etc. 
#include "debug.h"
#include "NMEA_GPS.hh"
#include "GTOP_utilities.h"

// how long are max NMEA lines to parse?
#define MAXLINELENGTH 120

// we double buffer: read one line in and leave one for the main program
static char line1[MAXLINELENGTH];
static char line2[MAXLINELENGTH];

// our index into filling the current line
static uint8_t lineidx=0;
// pointers to the double buffers
static char *currentline;
static char *lastline;

static bool inStandbyMode;


// General helper functions. =======================================
/**
 ******************************************************************
 *
 * Function Name :  DecodeTime
 *
 * Description : return a time_t structure based on GPS fix time
 *
 * Inputs : 
 *    p - character string input of fix time in format of HHMMSS.ss
 *    ms - return variable for milliseconds
 *    now - structure containing information for tm values (incomplete)
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 22-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static time_t DecodeTime(const char *p, float *ms, struct tm *now)
{
    SET_DEBUG_STACK;
    // Just GPS epoch.
    float    timef   = atof(p);   // UTC time reference to current day
    uint32_t time    = timef;
    uint8_t  hour    = time / 10000;
    uint8_t  minute  = (time % 10000) / 100;
    uint8_t  seconds = (time % 100);
    if (ms)
    {
	*ms =  fmod(timef, 1.0) * 1000;
    }
    if (now)
    {
	memset( now, 0, sizeof(struct tm));
	now->tm_sec  = seconds;
	now->tm_min  = minute;
	now->tm_hour = hour;
    }
    SET_DEBUG_STACK;
    return (seconds + 60.0*(minute + 60.0*hour));
}
/**
 ******************************************************************
 *
 * Function Name :  DecodeUTC
 *
 * Description : Decode date field from RMC message
 *
 * Inputs : 
 *     p   - charcter string containing date in the format DDMMYY
 *     now - struct tm from calling function to be filled. Assume this
 *           is partially filled from previous RMC field. 
 *
 * Returns : time_t in seconds from epoch
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
static time_t DecodeUTC(const char *p, struct tm *now)
{
    /*
     * Full epoch. assume previous function was used first. 
     * also assum input for p is ddmmyy
     * tm_sec 0-59
     * tm_min 0-59
     * tm_hour 0-23
     * tm_mday 1-31
     * tm_mon 0-11
     * tm_year number of years since 1900
     */
    SET_DEBUG_STACK;

    uint32_t fulldate = atof(p);
    now->tm_mday = fulldate / 10000;
    now->tm_mon  = (fulldate % 10000) / 100 - 1;
    now->tm_year = (fulldate % 100)+100;
    SET_DEBUG_STACK;
    return mktime(now);
}
static double DecodeDegMin(const char *p)
{
    SET_DEBUG_STACK;
    double  Degrees, Minutes, number;

    /* 
     * Format we are trying to parse is dddmm.mmmm 
     * I am going to do this vastly differently. 
     * I realize that on an embedded processor, floating point is 
     * a premium, not here though. 
     */
    number = atof(p);
    // Strip off fractional part immediately. 
    Minutes = modf( number, &Degrees);
    /*
     * The remainder of the minutes is embedded in the low two 
     * decimal places of the number variable at this point. 
     */
    Minutes = fmod(number, 100.0);
    // remove the integer part of the minutes. 
    Degrees = floor( Degrees/100.0);

    // Add in minutes portion
    Degrees += Minutes/60.0;

    SET_DEBUG_STACK;
    return (Degrees * DegToRad);   // Return radians
}

/**
 ******************************************************************
 *
 * Function Name :  
 *
 * Description : 
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
NMEA_POSITION::NMEA_POSITION(void)
{
    SET_DEBUG_STACK;
    fLatitude     = fLongitude = 0.0;
    fSeconds      = 0;
    fMilliseconds = 0.0;
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  
 *
 * Description : 
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
GGA::GGA(void) : NMEA_POSITION()
{
    SET_DEBUG_STACK;
    fGeoidheight  = 0.0;
    fAltitude     = 0.0;
    fFixIndicator = 0;
    fHDOP         = 0.0;
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  
 *
 * Description : 
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
bool GGA::Decode(const char *line)
{
    SET_DEBUG_STACK;
    clock_gettime( CLOCK_REALTIME, &fPCTime);

    char *p = (char *) line;

    // get time
    p = strchr(p, ',')+1;
    fSeconds = 0;
    fUTC     = DecodeTime( p, &fMilliseconds, NULL);

    // parse out latitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fLatitude = DecodeDegMin(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	if (p[0] == 'S') fLatitude *= -1.0;
    }
    
    // parse out longitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fLongitude = DecodeDegMin(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	if (p[0] == 'W') fLongitude *= -1.0;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fFixIndicator = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fSatellites = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fHDOP = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fAltitude = atof(p);
    }
    
    p = strchr(p, ',')+1;
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fGeoidheight = atof(p);
    }
    return true;
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  RMC Constructor
 *
 * Description : 
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
RMC::RMC(void) : NMEA_POSITION()
{
    SET_DEBUG_STACK;
    fSpeed  = 0.0;
    fCMG    = 0.0;
    fMagVariation = 0.0;
    fMode   = 'N';
    fDelta  = 0.0;
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  RMC decode
 *
 * Description :  decode an RMC message of format
 *     RMC - Recommende minimum sentance C
 *     HHMMSS.ss - time
 *     A - status (A)ctive or (V)oid
 *     DDMM.mmm  - Latitude
 *     N/S       - Latitude Hemisphere
 *     DDDMM.mmm - Longitude
 *     E/W       - East/west hemisphere
 *     Speed - Knots
 *     Heading
 *     DDMMYY    - date 
 *     Magnetic Variation
 *     Magnetic Variation Direction
 *     A - Mode indicator
 *        (A)utonomous
 *        (D)ifferential
 *        (E)stimated
 *        (F)loat RTK
 *        (M)anual input
 *        (N)o fix
 *        (P)recise
 *        (R)TK
 *        (S)imulator
 *     CHECKSUM
 *
 * Inputs : character line to decode. 
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
bool RMC::Decode(const char *line)
{
    const float k = 0.95;

    SET_DEBUG_STACK;
    double tmp;
    struct tm now;
    clock_gettime( CLOCK_REALTIME, &fPCTime);

    // found RMC
    char *p = (char *) line;

    // get time
    p = strchr(p, ',')+1;
    /*
     * Return the seconds into the UTC day AND
     * fill H,M,S portion of struct time.
     * The remainder will be filled below 
     */
    fUTC = DecodeTime( p, &fMilliseconds, &now);

    p = strchr(p, ',')+1;
    fMode = p[0];
#if 0
    //if (p[0] == 'A') 
//	fix = true;
    else if (p[0] == 'V')
	fix = false;
    else
	return false;
#endif
    // parse out latitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fLatitude = DecodeDegMin(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	if (p[0] == 'S') fLatitude *= -1.0;
    }
    
    // parse out longitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fLongitude = DecodeDegMin(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	if (p[0] == 'W') fLongitude *= -1.0;
    }
    // speed
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fSpeed = atof(p);
    }
    
    // angle
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	fCMG = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
	/*
	 * Pass in the partially filled UTC message, from above
	 * H,M,S should be filled. 
	 * This decode should provide DDMMYY
	 */
	fSeconds = DecodeUTC(p, &now);
	/* PC Time is local, convert to UTC */
	float dt   = (float) (fPCTime.tv_sec - fSeconds + timezone);
	dt  -= fMilliseconds/100.0;
	tmp     = fPCTime.tv_nsec;
	tmp     /= 1.0e9;
	dt  += tmp;
	/* Super simple LPF */
	fDelta = k*fDelta + (1.0-k)*dt;
    }
    SET_DEBUG_STACK;
    return true;
}
/**
 ******************************************************************
 *
 * Function Name :  
 *
 * Description : 
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
VTG::VTG(void)
{
    SET_DEBUG_STACK;
    fTrue       = 0.0;  // Course True
    fMagnetic   = 0.0;  // course magnetic
    fSpeedKnots = 0.0;
    fSpeedKPH   = 0.0; 
    fMode       = 'N';
    SET_DEBUG_STACK;
}
bool VTG::Decode(const char *line)
{
    SET_DEBUG_STACK;
    char *p = (char *) line;

    p = strchr(p, ',')+1;
    fTrue = atof(p);      // Course True

    p = strchr(p, ',')+1;
    if (*p != 'T')
	return false;

    p = strchr(p, ',')+1;
    fMagnetic = atof(p);  // course magnetic

    p = strchr(p, ',')+1;
    if (*p != 'M')
	return false;

    p = strchr(p, ',')+1;
    fSpeedKnots = atof(p);

    p = strchr(p, ',')+1;
    if (*p != 'N')
	return false;

    p = strchr(p, ',')+1;
    fSpeedKPH = atof(p); 

    p = strchr(p, ',')+1;
    if (*p != 'K')
	return false;

    p = strchr(p, ',')+1;
    fMode = *p;
    SET_DEBUG_STACK;
    return true;
}
GSA::GSA(void)
{
    SET_DEBUG_STACK;
    fMode1 = fMode2 = 'N';
    memset( fSatellite, 0, sizeof(fSatellite));
    fPDOP=fHDOP=fVDOP = 0.0;
    SET_DEBUG_STACK;
}
bool GSA::Decode(const char *line)
{
    SET_DEBUG_STACK;
    int  i;
    char *p = (char *) line;

    p = strchr(p, ',')+1;
    fMode1 = *p;

    p = strchr(p, ',')+1;
    fMode2 = atoi(p);

    for (i=0;i<12;i++)
    {
	p = strchr(p, ',')+1;
	fSatellite[i] = atoi(p);
    } 
    p = strchr(p, ',')+1;
    fPDOP = atof(p);

    p = strchr(p, ',')+1;
    fHDOP = atof(p);

    p = strchr(p, ',')+1;
    fVDOP = atof(p);
    SET_DEBUG_STACK;
    return true;
}


/**
 ******************************************************************
 *
 * Function Name : NMEA_GPS
 *
 * Description : Constructor
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
NMEA_GPS::NMEA_GPS( const char *port, speed_t BaudRate)
{
    SET_DEBUG_STACK;

    if(SerialOpen( port, BaudRate) < 0)
    {
	fErrorCode = SERIAL_OPEN_FAIL;
    }
    else
    {
	fErrorCode = ERROR_NONE;
    }

    // receive data
    paused      = false;
    lineidx     = 0;
    currentline = line1;
    lastline    = line2;
    fRMC = new RMC();
    fGGA = new GGA();
    fVTG = new VTG();
    fGSA = new GSA();
    memset(line1, 0, sizeof(line1));
    memset(line2, 0, sizeof(line2));
    SET_DEBUG_STACK;
}
NMEA_GPS::~NMEA_GPS(void)
{
    SET_DEBUG_STACK;
    CloseSerial();
    delete fRMC;
    delete fGGA;
    delete fVTG;
    SET_DEBUG_STACK;
}
bool NMEA_GPS::Read(void)
{
    SET_DEBUG_STACK;
    const struct timespec sleeptime = {0L, 100000000L};

    char c = 0;
    size_t n = read(GetSerial_fd(), &c, 1);
    if (n == 0)
    {
	nanosleep( &sleeptime, NULL);
	fRecvdflag = false;
    }
    else if (c == '\n') 
    {
	// Represents an end of line. 
	currentline[lineidx] = 0;
	// go ahead and decode what we have.
	parse(currentline);
	// setup for next set of data.
	if (currentline == line1) 
	{
	    currentline = line2;
	    lastline    = line1;
	} 
	else 
	{
	    currentline = line1;
	    lastline    = line2;
	}
	lineidx = 0;
	fRecvdflag = true;
    }
    else
    {
	currentline[lineidx++] = c;
	if (lineidx >= MAXLINELENGTH)
	    lineidx = MAXLINELENGTH-1;
	fRecvdflag = false;
    }
    SET_DEBUG_STACK;
    return fRecvdflag;
}
// read a Hex value and return the decimal equivalent
uint8_t NMEA_GPS::parseHex(char c) 
{
   SET_DEBUG_STACK;
   if (c < '0')
	return 0;
    if (c <= '9')
	return c - '0';
    if (c < 'A')
	return 0;
    if (c <= 'F')
	return (c - 'A')+10;
    // if (c > 'F')
    SET_DEBUG_STACK;
    return 0;
}

bool NMEA_GPS::CheckSum(const char *line)
{
    SET_DEBUG_STACK;

    /*
     * do checksum check
     * first look if we even have one
     */
    if (line[strlen(line)-4] == '*') 
    {
	uint16_t sum = parseHex(line[strlen(line)-3]) * 16;
	sum += parseHex(line[strlen(line)-2]);
    
	// check checksum 
	for (uint32_t i=2; i < (strlen(line)-4); i++) 
	{
	    sum ^= line[i];
	}
	if (sum != 0) 
	{
	    // bad checksum :(
	    return false;
	}
    }
    SET_DEBUG_STACK;
    return true;
}
bool NMEA_GPS::parse(const char *nmea) 
{
    SET_DEBUG_STACK;
    bool    rc = false;

    /*
     * if possible need VTG, XTC, WPL, APB- auto pilot b
     */

    // look for a few common sentences
    if (strstr(nmea, "$GPGGA")) {
	// found GGA
	fLastID = MESSAGE_GGA; // Position
	rc = fGGA->Decode(nmea);
    }
    else if (strstr(nmea, "$GPRMC")) 
    {
	fLastID = MESSAGE_RMC; // Recommended navigation data.
	rc = fRMC->Decode(nmea);
    }
    else if (strstr(nmea, "$GPVTG"))
    {
	// Course and speed. 
	fLastID = MESSAGE_VTG;
	rc = fVTG->Decode(nmea);
    }
    else if (strstr(nmea, "$GPGSA"))
    {
	// Active satellite data.
	fLastID = MESSAGE_GSA;
	rc = fGSA->Decode(nmea);
    }
    else if (strstr(nmea, "$GPGSV"))
    {
	// GNSS Satellites in view.
	fLastID = MESSAGE_GSV;
	rc = true;
    }
    else if (strstr( nmea, PMTK_LOCUS_STARTSTOPACK))
    {
	fLastID = MESSAGE_LOG;
	rc = true;
    }
    else if (strstr( nmea, PMTK_AWAKE))
    {
	fLastID = MESSAGE_AWAKE;
	rc = true;
    }
    SET_DEBUG_STACK;

    return rc;
}

void NMEA_GPS::sendCommand(const char *str) 
{
    SET_DEBUG_STACK;
    write(GetSerial_fd(), str, strlen(str));
    SET_DEBUG_STACK;
}


void NMEA_GPS::LOCUS_StartLogger(void) 
{
    SET_DEBUG_STACK;
    fRecvdflag = false;
    sendCommand(PMTK_LOCUS_STARTLOG);
}

void NMEA_GPS::LOCUS_StopLogger(void) 
{
    SET_DEBUG_STACK;
    fRecvdflag = false;
    sendCommand(PMTK_LOCUS_STOPLOG);
}

void NMEA_GPS::LOCUS_ReadStatus(void) 
{
    SET_DEBUG_STACK;
    sendCommand(PMTK_LOCUS_QUERY_STATUS);
}

PMTKLOG::PMTKLOG(void)
{
    SET_DEBUG_STACK;
    fLOCUS_serial = fLOCUS_records = 0;
    fLOCUS_type =fLOCUS_mode = fLOCUS_config = fLOCUS_interval = 0;
    fLOCUS_distance = fLOCUS_speed = fLOCUS_status = fLOCUS_percent= 0.0;
}
bool PMTKLOG::Decode(const char *line)
{ 
    SET_DEBUG_STACK;
    uint8_t i;
    uint16_t parsed[10];
    char *p = (char *) line;

    for (i=0;i<10;i++) parsed[i] = -1;

    char *response = strchr(p, ',');
    for (i=0; i<10; i++) {
	if (!response || (response[0] == 0) || (response[0] == '*')) 
	    break;
	response++;
	parsed[i]=0;
	while ((response[0] != ',') && 
	       (response[0] != '*') && (response[0] != 0)) {
	    parsed[i] *= 10;
	    char c = response[0];
	    if (isdigit(c))
		parsed[i] += c - '0';
	    else
		parsed[i] = c;
	    response++;
	}
    }
    fLOCUS_serial = parsed[0];
    fLOCUS_type = parsed[1];
    if (isalpha(parsed[2])) {
	parsed[2] = parsed[2] - 'a' + 10; 
    }
    fLOCUS_mode     = parsed[2];
    fLOCUS_config   = parsed[3];
    fLOCUS_interval = parsed[4];
    fLOCUS_distance = parsed[5];
    fLOCUS_speed    = parsed[6];
    fLOCUS_status   = !parsed[7];
    fLOCUS_records  = parsed[8];
    fLOCUS_percent  = parsed[9];
    return true;
}

// Standby Mode Switches
bool NMEA_GPS::standby(void) 
{
    SET_DEBUG_STACK;
    if (inStandbyMode) 
    {
        // Returns false if already in standby mode, so that you 
	// do not wake it up by sending commands to GPS
	return false;  
    }
    else 
    {
	inStandbyMode = true;
	sendCommand(PMTK_STANDBY);
        /*
	 * return waitForSentence(PMTK_STANDBY_SUCCESS);  
	 * don't seem to be fast enough to catch the message, 
	 * or something else just is not working
	 */
	return true;
    }
}

bool NMEA_GPS::wakeup(void) 
{
    SET_DEBUG_STACK;
    //UNFINISHED
    if (inStandbyMode) 
    {
	inStandbyMode = false;
	sendCommand("");  // send byte to wake it up
	return waitForSentence(PMTK_AWAKE);
    }
    else 
    {
	return false;  // Returns false if not in standby mode, nothing to wakeup
    }
}

char *NMEA_GPS::lastNMEA(void) 
{
    SET_DEBUG_STACK;
    fRecvdflag = false;
    return (char *)lastline;
}
bool NMEA_GPS::newNMEAreceived(void) 
{
    SET_DEBUG_STACK;
    return fRecvdflag;
}

bool NMEA_GPS::waitForSentence(const char *wait4me, uint8_t max) 
{
    SET_DEBUG_STACK;
    char str[20];
    // This is unfinished, don't use it. 
    uint8_t i=0;
    while (i < max) {
	if (newNMEAreceived()) { 
	    char *nmea = lastNMEA();
	    strncpy(str, nmea, 20);
	    str[19] = 0;
	    i++;

	    if (strstr(str, wait4me))
		return true;
	}
    }
    SET_DEBUG_STACK;

    return false;
}
