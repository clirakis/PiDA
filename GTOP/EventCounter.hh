/**
 ******************************************************************
 *
 * Module Name : EventCounter.hh
 *
 * Author/Date : C.B. Lirakis / 20-Dec-23
 *
 * Description : 
 *    Create an event counter with a shared memory that can be read
 *    by other modules providing a unique number idenitifier 
 *    to be used with matching data streams. This might be a 
 *    little simplier than using time(&now)
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
#ifndef __EVENTCOUNTER_hh_
#define __EVENTCOUNTER_hh_
#include <cstdint>
#  include "SharedMem2.hh"
/// EventCounter documentation here. 
class EventCounter : public SharedMem2
{
public:
    /*! @brief Default Constructor
     */
    EventCounter(bool Server=false);
    /// Default destructor
    ~EventCounter();

    uint32_t Count(void);  
    uint32_t Increment(void);

    /// EventCounter function
    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    //void* function(const char *Name);
private:
    bool     fServer;
    uint32_t fCount;
};
#endif
