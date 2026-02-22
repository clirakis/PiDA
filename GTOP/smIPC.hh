/**
 ******************************************************************
 *
 * Module Name : smIPC.hh
 *
 * Author/Date : C.B. Lirakis / 22-Feb-22
 *
 * Description : Inialize the IPC, in this case a shared memory IPC. 
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
 * 07-Mar-19  CBL  Made into class
 * 21-Sep-20  CBL  Changed to SharedMem2
 * 20-Feb-22  CBL  Changed to NMEA GPS
 * 22-Feb-26  CBL  Took out the filename SM and changed the
 *                 command structure to allow for 512 bytes of string
 *                 data to be returned. Use this when querying filename
 *
 * Classification : Unclassified
 *
 * References : NONE
 *
 *******************************************************************
 */
#ifndef __SMIPC_hh_
#define __SMIPC_hh_
#   include "NMEA_GPS.hh"
#   include "CObject.hh"
#   include "SharedMem2.hh"

class GPS_IPC : public CObject 
{
public:
    /*! Constructor */
    GPS_IPC(void);
    /*! Destructor */
    ~GPS_IPC(void);
    /*! Send the data */
    void Update(void);
    /*! provide shared memory for inbound commands. */
    void ProcessCommands(void);

private:
    /**
     * Shared memory for position data. GPGGA
     */
    SharedMem2   *pSM_PositionData;
    /**
     * Shared memory for Solution Data GPGSA
     */
    SharedMem2   *pSM_SolutionData;
    /**
     * Shared memory for RMC message, contains delta
     */
    SharedMem2   *pSM_Minimum;
    /**
     * Shared memory for velocity data. GPVTG
     */
    SharedMem2   *pSM_VelocityData;

    SharedMem2   *pSM_Commands;        // A way to communicate with remote
};
#endif
