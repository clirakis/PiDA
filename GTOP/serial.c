/**
 ******************************************************************
 *
 * Module Name : serial.c
 *
 * Author/Date : C.B. Lirakis / 24-Dec-05
 *
 * Description : Handle all serial I/O here
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
// System includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/// Local Includes.
#include "serial.h"
#include "debug.h"

static int serial_fd;

int GetSerial_fd(void)
{
    return serial_fd;
}
/**
 ******************************************************************
 *
 * Function Name : SerialOpen
 *
 * Description : Open the provided serial port
 *
 * Inputs : port - character name of device to open
 *          BaudRate - the baud rate we want to use. 
 *
 * Returns : serial fd
 *
 * Error Conditions : oh there are several
 * 
 * Unit Tested on: 15-Nov-25
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
int SerialOpen(const char *port, speed_t BaudRate)
{
    int rc = -1;
    struct termios newtio;


    /*
     * setup for text read.
     * speed 9600 baud; rows 0; columns 0; line = 0;
     * intr = <undef>; quit = <undef>; erase = <undef>; kill = <undef>; eof = ^D;
     * eol = <undef>; eol2 = <undef>; swtch = <undef>; start = <undef>; stop = <undef>;
     * susp = <undef>; rprnt = <undef>; werase = <undef>; lnext = <undef>;
     * flush = <undef>; min = 1; time = 5;
     * -parenb -parodd -cmspar cs8 -hupcl -cstopb cread clocal crtscts
     * ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff
     * -iuclc -ixany -imaxbel -iutf8
     * -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
     * -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt
 
     */
    if (port != NULL)
    {
	rc = open( port, O_RDWR | O_NOCTTY | O_NDELAY);	
	if (rc <0)
	{
	    printf("%s", strerror(errno));
	}
	else
	{
	    serial_fd = rc;
	    /* 
	     * See page 354 of Steven's Advanced UNIX Programming book. 
	     * Get existing termios properties. 
	     * 
	     * Alternative reference is:
	     * Posix Programmer's Guide - Donald A. Lewine
	     * c 1991 O'Oreily and Associates
	     * page 153  pertains  to Termial  I/O
	     */

	    /* clear struct for new port settings */
	    memset(&newtio,0, sizeof(newtio)); 
	    tcgetattr( serial_fd, &newtio);
            cfsetispeed( &newtio, BaudRate);
            cfsetospeed( &newtio, BaudRate);

	    /*
	     * Control flag
	     *
	     * CRTSCTS : output hardware flow control (only used if 
	     * the cable has all necessary lines. See sect. 7 of Serial-HOWTO)
	     * CS8     : 8n1 (8bit,no parity,1 stopbit)
	     * CLOCAL  : local connection, no modem contol
	     * CREAD   : enable receiving characters
	     * 28-Dec-15
	     * PARENB  : enable partiy on output
	     * PARODD  : odd parity. 
	     */
//	    newtio.c_cflag = CS8 | CLOCAL | CREAD | PARENB | PARODD;
//	    newtio.c_cflag = CS8 | CLOCAL | CREAD | CRTSCTS;

	    /*
	     * Input modes.
	     * IGNPAR  : ignore bytes with parity errors
	     * ICRNL   : map CR to NL (otherwise a CR input on the other 
	     *           computer will not terminate input)
	     *         otherwise make device raw (no other input processing)
	     */
	    newtio.c_iflag = ICRNL | IGNBRK;
	    /*
	     * Output modes
	     * Raw output.
	     */
	    newtio.c_oflag = NL0 | CR0 | TAB0 | BS0 | VT0| FF0;
	    /*
	     * ICANON  : enable canonical input
	     *           disable all echo functionality, and don't 
	     *           send signals to the calling program.
	     */
	    newtio.c_lflag = ~ICANON;

	    /*
	     * Initialize all control characters
	     * default values can be found in /usr/include/termios.h, 
	     * and are given in the comments, but we don't need them here
	     */
	    newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	    newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	    newtio.c_cc[VERASE]   = 0;     /* del */
	    newtio.c_cc[VKILL]    = 0;     /* @ */
	    newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
	    newtio.c_cc[VTIME]    = 5;     /* inter-character timer unused */
	    newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	    //    newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	    newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	    newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	    newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	    newtio.c_cc[VEOL]     = 0;     /* '\0' */
	    newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	    newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	    newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	    newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	    newtio.c_cc[VEOL2]    = 0;     /* '\0' */
	    /*
	     * now clean the modem line and activate the settings for the port
	     */
	    tcflush(rc, TCIFLUSH);
	    tcsetattr(rc, TCSANOW, &newtio);
	    /* 
	     * this makes the read() function wait until it has stuff to 
	     * read before reading 
	     */
	    fcntl( rc, F_SETFL, 0);
	}
    }
    return rc;
}
/**
 ******************************************************************
 *
 * Function Name : Close Serial
 *
 * Description : close the serial file descriptor
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 15-Nov-25
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void CloseSerial(void)
{
    close(serial_fd);
}
