/**
 * *********************************
This is the Adafruit GPS library - the ultimate GPS library
for the ultimate GPS module!

Tested and works great with the Adafruit Ultimate GPS module
using MTK33x9 chipset
    ------> http://www.adafruit.com/products/746
Pick one up today at the Adafruit electronics shop 
and help support open source hardware & software! -ada

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above must be included in any redistribution
*
* Modified to work on PI. 
****************************************/
// Fllybob added lines 34,35 and 40,41 to add 100mHz logging capability 
// 20-Feb-22 CBL Modified to add in features for output, datasize and pointers

#ifndef _NMEA_GPS_HH_
#define _NMEA_GPS_HH_
#include <stdint.h>
#include <time.h>
#include "serial.h"

/**
 * different commands to set the update rate from once a second 
 * (1 Hz) to 10 times a second (10Hz) Note that these only control 
 * the rate at which the position is echoed, to actually speed up the
 * position fix you must also send one of the position fix rate commands 
 * below too.
 */
/* Once every 10 seconds, 100 milliHertz. */
#define PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ  "$PMTK220,10000*2F" 
// Once every 5 seconds, 200 millihertz.
#define PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ  "$PMTK220,5000*1B"  
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"
// Position fix update rate commands.
// Once every 10 seconds, 100 millihertz.
#define PMTK_API_SET_FIX_CTL_100_MILLIHERTZ  "$PMTK300,10000,0,0,0,0*2C" 
// Once every 5 seconds, 200 millihertz.
#define PMTK_API_SET_FIX_CTL_200_MILLIHERTZ  "$PMTK300,5000,0,0,0,0*18"  
#define PMTK_API_SET_FIX_CTL_1HZ  "$PMTK300,1000,0,0,0,0*1C"
#define PMTK_API_SET_FIX_CTL_5HZ  "$PMTK300,200,0,0,0,0*2F"
// Can't fix position faster than 5 times a second!


#define PMTK_SET_BAUD_57600 "$PMTK251,57600*2C"
#define PMTK_SET_BAUD_9600 "$PMTK251,9600*17"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
// turn on GPRMC and GGA
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn on ALL THE DATA
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn off output
#define PMTK_SET_NMEA_OUTPUT_OFF "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

/*
 * to generate your own sentences, check out the MTK command 
 * datasheet and use a checksum calculator such as the awesome 
 * http://www.hhhh.org/wiml/proj/nmeaxor.html
 */

#define PMTK_LOCUS_STARTLOG  "$PMTK185,0*22"
#define PMTK_LOCUS_STOPLOG "$PMTK185,1*23"
#define PMTK_LOCUS_STARTSTOPACK "$PMTK001,185,3*3C"
#define PMTK_LOCUS_QUERY_STATUS "$PMTK183*38"
#define PMTK_LOCUS_ERASE_FLASH "$PMTK184,1*22"
#define LOCUS_OVERLAP 0
#define LOCUS_FULLSTOP 1

#define PMTK_ENABLE_SBAS "$PMTK313,1*2E"
#define PMTK_ENABLE_WAAS "$PMTK301,2*2E"

// standby command & boot successful message
#define PMTK_STANDBY "$PMTK161,0*28"
#define PMTK_STANDBY_SUCCESS "$PMTK001,161,3*36"  // Not needed currently
#define PMTK_AWAKE "$PMTK010,002*2D"

// ask for the release and version
#define PMTK_Q_RELEASE "$PMTK605*31"

// request for updates on antenna status 
#define PGCMD_ANTENNA "$PGCMD,33,1*6C" 
#define PGCMD_NOANTENNA "$PGCMD,33,0*6D" 

// how long to wait when we're looking for a response
#define MAXWAITSENTENCE 5

    // Fixed point latitude and longitude value with degrees stored in 
    // units of 1/100000 degrees,
    // and minutes stored in units of 1/100000 degrees.  
    // See pull #13 for more details:
    //   https://github.com/adafruit/NMEA-GPS-Library/pull/13

class NMEA_POSITION {
public:
    /** Constructor */
    NMEA_POSITION(void);
    /** Return fix latitude in radians. */
    inline float  Latitude(void)  const {return fLatitude;};
    /** Return fix longitude in radians */
    inline float  Longitude(void) const {return fLongitude;};
    /** 
     * Epoch Seconds 
     */
    inline time_t Seconds(void)   const {return fSeconds;};
    /** Milliseconds on epoch */
    inline float  Milli(void)     const {return fMilliseconds;};
    /** UTC of current day seconds */
    inline time_t UTC(void)       const {return fUTC;};

    /* THINGS USED to relay in shared memory. --------------------- */
    /*! Get a pointer to the beginning of the data storage. */
    inline void* DataPointer(void) {return (void*)&fPCTime;};
    /*! Return the overall data size for the structure. */
    inline static size_t DataSize(void) 
	{return (sizeof(struct timespec)+3*sizeof(float)+2*sizeof(time_t));};

    /*! operator overload to output contents of class for inspection
     * this data is in character format. 
     */
    friend ostream& operator<<(ostream& output, const NMEA_POSITION &n); 

protected:
    struct timespec fPCTime;
    float  fLatitude, fLongitude; // In radians. 
    time_t fSeconds;              // seconds in epoch
    time_t fUTC;                  // seconds in current day, UTC
    float  fMilliseconds;         // ms on time of fix
};

class GGA : public NMEA_POSITION
{
public:
    GGA(void);
    bool Decode(const char *);
    /**
     * Height of geoid (mean sea level) above WGS84 ellipsoid
     */
    inline float Geoid(void)      const {return fGeoidheight;};
    /**
     * altitude in meters above MSL (geoid)
     */
    inline float Altitude(void)   const {return fAltitude;};
    /** 
     * Fix Indicator made up of:
     *   0 - Invalid
     *   1 GNSS fix (SPS)
     *   2 DGPS fix
     *   3 PPS  fix
     *   4 RTK
     *   5 - UNASSIGNED
     *   6 - DR
     *   7 - Manual input mode
     *   8 - Simulation Mode
     */
    inline char  Fix(void)        const {return fFixIndicator;};
    /**
     * Number of satellites in used in fix. 
     */
    inline char  Satellites(void) const {return fSatellites;};
    /**
     * Horizontal dilution of position
     */
    inline float HDOP(void)       const {return fHDOP;};

    /* THINGS USED to relay in shared memory. --------------------- */
    /*! Get a pointer to the beginning of the data storage. */
    inline void* DataPointer(void) {return (void*)&fPCTime;};

    /*! 
     * Return the overall data size for the structure. 
     */
    inline static size_t DataSize(void) 
	{return (4*sizeof(uint8_t)+4*sizeof(float) + NMEA_POSITION::DataSize());};

    /*! operator overload to output contents of class for inspection
     * this data is in character format. 
     */
    friend ostream& operator<<(ostream& output, const GGA &n); 

private:
    /** Height of Geoid above mean sea level WGS84 */
    float   fGeoidheight;  
    /** altitude relative to mean geoid */ 
    float   fAltitude;
    /** 
     * Fix Indicator made up of:
     *   0 - Invalid
     *   1 GNSS fix (SPS)
     *   2 DGPS fix
     *   3 PPS  fix
     *   4 RTK
     *   5 - UNASSIGNED
     *   6 - DR
     *   7 - Manual input mode
     *   8 - Simulation Mode
     */
    uint8_t fFixIndicator; 
    uint8_t fSatellites;   // number in fix.
    uint8_t fspace1;
    uint8_t fspace2;
    float   fHDOP;
};

/**
 * RMC Recommended minimum information class
 */
class RMC : public NMEA_POSITION
{
public:
    RMC(void);
    bool Decode(const char *);
    inline float Speed(void)  const {return fSpeed;};
    inline float CMG(void)    const {return fCMG;};
    inline float MagVar(void) const {return fMagVariation;};
    inline char  Mode(void)   const {return fMode;};
    inline float Delta(void)  const {return fDelta;};

    /*! Get a pointer to the beginning of the data storage. */
    inline void* DataPointer(void) {return (void*)&fPCTime;};

    /*! Return the overall data size for the structure. */
    inline static size_t DataSize(void) 
	{return (4*sizeof(float)+4*sizeof(char) + NMEA_POSITION::DataSize());};

    /*! operator overload to output contents of class for inspection
     * this data is in character format. 
     */
    friend ostream& operator<<(ostream& output, const RMC &n); 

private:
    /** difference between PC time and GPS time  in seconds */
    float  fDelta;
    /**
     * Speed in knots
     */
    float  fSpeed;
    /**
     * Course made good
     */
    float  fCMG; 
    /**
     * Magnetic Variation. 
     */
    float  fMagVariation;
    /**
     * FIX MODE
     */
    char   fMode;
    char   fPlaceholder[3];
};

// Active satellites in solution.
class GSA 
{
public:
    GSA(void);
    bool Decode(const char *);
    inline char Mode1(void) const {return fMode1;};
    inline char Mode2(void) const {return fMode2;};
    inline unsigned char ID(int i) const {return fSatellite[i%12];};
    inline const unsigned char* IDS(void) {return fSatellite;};
    inline float PDOP(void) const {return fPDOP;};
    inline float HDOP(void) const {return fHDOP;};
    inline float VDOP(void) const {return fVDOP;};
    inline float TDOP(void) const {return 0.0;};

    /*! Get a pointer to the beginning of the data storage. */
    inline void* DataPointer(void) {return (void*)&fPDOP;};

    /*! Return the overall data size for the structure. */
    inline static size_t DataSize(void) 
	{return (4*sizeof(float)+16*sizeof(char));};

    /*! operator overload to output contents of class for inspection
     * this data is in character format. 
     */
    friend ostream& operator<<(ostream& output, const GSA &n); 

private:
    float         fPDOP, fHDOP, fVDOP, fSpace;
    char          fMode1, fMode2;
    char          fSpace1, fSpace2; // Placeholders
    unsigned char fSatellite[12]; 
};
#if 0
// Satellites in view
class SatelliteData
{
public: 
    SatelliteData(void);
    bool Decode(const char *);
private:
    unsigned char fID;
    unsigned char fElevation;
    unsigned char fAzimuth;
    unsigned char fSNR;
}
class GSV
{
public:
    GSV(void);
    bool Decode(const char *);
private:
    unsigned char fNMsg;
    SatelliteData fSat[12];
};
#endif
// Course and speed
class VTG
{
public:
    VTG(void);
    bool Decode(const char *);
    inline float True(void)  const {return fTrue;};
    inline float Mag(void)   const {return fMagnetic;};
    inline float Knots(void) const {return fSpeedKnots;};
    inline float KPH(void)   const {return fSpeedKPH;};
    inline int   Mode(void)  const {return (int) fMode;};

    /*! Get a pointer to the beginning of the data storage. */
    inline void* DataPointer(void) {return (void*)&fTrue;};

    /*! Return the overall data size for the structure. */
    inline static size_t DataSize(void) 
	{return (4*sizeof(float)+4*sizeof(char));};

    /*! operator overload to output contents of class for inspection
     * this data is in character format. 
     */
    friend ostream& operator<<(ostream& output, const VTG &n); 

private:
    float fTrue;      // Course True
    float fMagnetic;  // course magnetic
    float fSpeedKnots;
    float fSpeedKPH; 
    char  fMode;      // Autonomous, Differential, Estimated
    char  fSpace[3];  // round out to even 4 byte boundry
};


class PMTKLOG 
{
public:
    PMTKLOG(void);
    bool Decode(const char *);
    inline uint16_t Serial(void)   {return fLOCUS_serial;};
    inline uint16_t Records(void)  {return fLOCUS_records;};
    inline uint8_t  Type(void)     {return fLOCUS_type;};
    inline uint8_t  Mode(void)     {return fLOCUS_mode;};
    inline uint8_t  Config(void)   {return fLOCUS_config;};
    inline uint8_t  Interval(void) {return fLOCUS_interval;};
    inline uint8_t  Distance(void) {return fLOCUS_distance;};
    inline uint8_t  Speed(void)    {return fLOCUS_speed;};
    inline uint8_t  Status(void)   {return fLOCUS_status;};
    inline uint8_t  Percent(void)  {return fLOCUS_percent;};

private:
    uint16_t fLOCUS_serial, fLOCUS_records;
    uint8_t  fLOCUS_type, fLOCUS_mode, fLOCUS_config, fLOCUS_interval;
    uint8_t  fLOCUS_distance, fLOCUS_speed, fLOCUS_status, fLOCUS_percent;
};


class NMEA_GPS 
{
 public:
    enum ErrorCodes{ERROR_NONE=0, SERIAL_OPEN_FAIL};
    enum MessageID {MESSAGE_NONE=0,MESSAGE_GGA, MESSAGE_RMC, MESSAGE_VTG, 
		    MESSAGE_GSA, MESSAGE_GSV, MESSAGE_LOG, MESSAGE_AWAKE};
 
    // Constructor - calls serial port open for us. 
    NMEA_GPS( const char *port, speed_t BaudRate);
    ~NMEA_GPS(void);

    inline int  ErrorCode(void) {return fErrorCode;};
    inline bool Error(void)     {return !(fErrorCode == ERROR_NONE);};
    inline int  LastID(void)    {return fLastID;};
    inline GGA* pGGA(void)      {return fGGA;};
    inline RMC* pRMC(void)      {return fRMC;};
    inline VTG* pVTG(void)      {return fVTG;};
    inline GSA* pGSA(void)      {return fGSA;};

    // See if anything is available. 
    bool Read(void);

    bool waitForSentence(const char *wait, uint8_t max = MAXWAITSENTENCE);
    void LOCUS_StartLogger(void);
    void LOCUS_StopLogger(void);
    void LOCUS_ReadStatus(void);
    
    char *lastNMEA(void);
    bool newNMEAreceived();
    
    void sendCommand(const char *);
    
    void pause(bool b);
    
    bool wakeup(void);
    bool standby(void);
    
private:
    // Private funciton calls. 
    uint8_t parseResponse(char *response);
    uint8_t parseHex(char c);
    bool parseNMEA(char *response);    
    bool parse(const char *);
    bool CheckSum(const char *line);
    bool ParseGGA(const char *line);
    bool ParseRMC(const char *line);

    // Private variables
    GGA  *fGGA;
    RMC  *fRMC;
    VTG  *fVTG;
    GSA  *fGSA;

    bool paused;    
    int  fErrorCode;
    int  fLastID;
    bool fRecvdflag;
};


#endif
