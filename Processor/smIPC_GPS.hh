/**
 ******************************************************************
 *
 * Module Name : smIPC_GPS.hh
 *
 * Author/Date : C.B. Lirakis / 23-Feb-22
 *
 * Description : Inialize the IPC for read, in this case a shared memory IPC. 
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
 * Classification : Unclassified
 *
 * References : NONE
 *
 *******************************************************************
 */
#ifndef __SMIPC_GPS_hh_
#define __SMIPC_GPS_hh_
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

    /*! Receive the data, Returns true if new data was available. */
    bool Update(void);

    /** access RMC */
    inline RMC* GetRMC(void) {return fRMC;};

    /** access GGA */
    inline GGA* GetGGA(void) {return fGGA;};

    /** access  VTG */
    inline VTG* GetVTG(void) {return fVTG;};

    /** access  GSA*/
    inline GSA* GetGSA(void) {return fGSA;};

private:

    RMC        *fRMC;
    GGA        *fGGA;
    VTG        *fVTG;
    GSA        *fGSA;

    /**
     * Shared memory for position data. GPGGA
     */
    SharedMem2   *pSM_PositionData;
    /**
     * Shared memory for Solution Data GPGSA
     */
    SharedMem2   *pSM_SolutionData;
    /**
     * Shared memory for velocity data. GPVTG
     */
    SharedMem2   *pSM_VelocityData;
    /**
     * SharedMemory with GPS time and delta. 
     */
    SharedMem2   *pSM_Minimum;
};
#endif
