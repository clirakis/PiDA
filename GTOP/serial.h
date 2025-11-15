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
      /*!
       * @brief open the serial port
       * @param port - character name describing path and port name. 
       * @param BaudRate - 
       */
      int SerialOpen( const char *port, speed_t BaudRate);
      /*!
       * @brief GetSerial_fd - return the integer number associated with the 
       *                       serial port. 
       */
      int GetSerial_fd(void);
      /*!
       * @brief CloseSerial
       * Close the serial port down. 
       */
      void CloseSerial();
# ifdef __cplusplus
  }
# endif
#endif
