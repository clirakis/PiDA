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
#   include "ICM-20948.hh"
#   include "SharedMem2.hh"
#   include "NMEA_GPS.hh"   // to get position data. 

class IMU_IPC : public CObject 
{
public:
    /*! Constructor */
    IMU_IPC(void);
    /*! Destructor */
    ~IMU_IPC(void);
    /*! Send the data */
    void Update(void);

    /*! Update filename in shared memory. */
    void UpdateFilename(const char *name);

    GGA *GetPosition(void) const;

private:
    /**
     * Shared memory for IMU, write
     */
    SharedMem2   *pSM;
    /**
     * Position shared memory, read. 
     */
    SharedMem2   *pSM_Position;

    /**
     * Shared memory segment for current data file name. 
     */
    SharedMem2   *fSM_Filename;

    GGA          *fGGA;
};
#endif
