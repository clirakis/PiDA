/**
 ******************************************************************
 *
 * Module Name : serial.h
 *
 * Author/Date : C.B. Lirakis / 24-Dec-05
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
 *******************************************************************
 */
#ifndef __SERIAL_h_
#define __SERIAL_h_
#include <termios.h>
# ifdef __cplusplus
  extern          "C"
  {
# endif
      int SerialOpen( const char *port, speed_t BaudRate);
      int GetSerial_fd(void);
      void CloseSerial();
# ifdef __cplusplus
  }
# endif
#endif
