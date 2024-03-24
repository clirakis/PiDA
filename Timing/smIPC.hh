/**
 ******************************************************************
 *
 * Module Name : smIPC.hh
 *
 * Author/Date : C.B. Lirakis / 27-Feb-22
 *
 * Description : Inialize the IPC, in this case a shared memory IPC. 
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : NONE
 *
 *******************************************************************
 */
#ifndef __SMIPC_hh_
#define __SMIPC_hh_
#   include "SharedMem2.hh"
#   include "NMEA_GPS.hh"   // to get position data. 

class TIMING_IPC : public CObject 
{
public:
    /*! Constructor */
    TIMING_IPC(void);
    /*! Destructor */
    ~TIMING_IPC(void);
    void Update(void);
    GGA *GetPosition(void) const;

private:
    /**
     * Shared memory for TIMING, write
     */
    SharedMem2   *pSM;
    /**
     * Position shared memory, read. 
     */
    SharedMem2   *pSM_Position;
    GGA          *fGGA;
};
#endif
