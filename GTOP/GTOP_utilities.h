/**
 ******************************************************************
 *
 * Module Name : tsip_utilities.h
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
 *
 *
 *******************************************************************
 */
#ifndef __GTOPUTILITIES_h_
#define __GTOPUTILITIES_h_
#include <time.h>
#include <math.h>

# ifdef __cplusplus
  extern          "C"
  {
# endif
      struct timespec DateFromGPSTime(float TimeOfWeek, int WeekNumber, 
				      float UTC_Offset);
      void ClearBit(unsigned char *mybyte,unsigned char mask);
      void SetBit(unsigned char *mybyte,unsigned char mask);
      void ToggleBit(unsigned char *mybyte,unsigned char mask);

# ifdef __cplusplus
  }
# endif
#endif
