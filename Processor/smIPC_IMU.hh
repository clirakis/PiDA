/**
 ******************************************************************
 *
 * Module Name : smIPC_IMU.hh
 *
 * Author/Date : C.B. Lirakis / 27-Feb-22
 *
 * Description : Inialize the IPC, in this case a shared memory IPC. 
 * This is the receive side. 
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
#ifndef __SMIPC_IMU_hh_
#define __SMIPC_IMU_hh_
#   include "ICM-20948.hh"
#   include "SharedMem2.hh"
#   include "IMUData.hh"

class IMU_IPC : public CObject 
{
public:
    /*! Constructor */
    IMU_IPC(void);
    /*! Destructor */
    ~IMU_IPC(void);
    /*! Get the data */
    bool Update(void);

    /*! Get filename in shared memory. */
    void UpdateFilename(void);

    /*! Access the IMU data stream */
    inline IMUData* GetIMU(void) {return fIMU;};

private:
    /**
     * Shared memory for position data. GPGGA
     */
    SharedMem2   *pSM;
    IMUData      *fIMU;

    /**
     * Shared memory segment for current data file name. 
     */
    SharedMem2   *fSM_Filename;
};
#endif
