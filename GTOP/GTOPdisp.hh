/**
 ******************************************************************
 *
 * Module Name : GTOPdisp.hh
 *
 * Author/Date : C.B. Lirakis / 24-Dec-15
 *
 * Description : display relevant data about GTOP GPS receiver
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * 19-Feb-22   CBL Made into class. 
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
#ifndef __GTOPDISP_h__
#define __GTOPDISP_h__
#include <time.h>
#include <stdint.h>
#include <ncurses.h>

#include "NMEA_GPS.hh"

/* Screen Layout */
/* Row where various messages are displayed. */
#define TOP_BAR        2
#define STATUS_AREA    4
#define STATUS_HEIGHT 11
#define MESSAGE_AREA  20
#define COMMAND_AREA  22

/* Columns */
#define LEFT_AREA     21
#define RIGHT_AREA    LEFT_AREA+21


/* Screen definitions. */
#define POSITION_SCREEN     1
#define SV_SCREEN           POSITION_SCREEN+1 
#define OPTIONS_SCREEN      SV_SCREEN+1 
#define HELP_SCREEN         OPTIONS_SCREEN+1
#define TEST_SCREEN         HELP_SCREEN+1
#define FILTER_SCREEN       TEST_SCREEN+1
#define PROCESSING_SCREEN   FILTER_SCREEN+1

class GTOP_Display 
{
public:
    /**
     * Constructor
     */
    GTOP_Display(void);

    /**
     * Desctuctor
     */
    ~GTOP_Display(void);

    /** 
     * Is the display running?
     */
    inline bool IsRunning(void) const {return fRun;};

    /**
     * command the display thread to exit. 
     */
    inline void Stop(void) {fRun = false;};


    /**
     * paint the main frame on the terminal. 
     */
    void main_frame(void);


    /**
     *
     */
    void ParseHomeKeys( char c);


    /* Options screen stuff */
    /*! Paint the options screen. */
    void OptionsScreen(void);
    /*! Parse the keys associated with the options screen */
    void ParseOptionsKeys(char c);
    void paint_options(int incol, 
		       unsigned char pos_code, unsigned char vel_code,
		       unsigned char time_code, unsigned char aux_code);
    void copy_options(unsigned char pos_code, unsigned char vel_code,
		      unsigned char time_code, unsigned char aux_code);


    /**
     * Call this to update the display when the frame is complete. 
     */
    void Update(NMEA_GPS *);

    void WriteMsgToScreen(const char *s);
    int  checkKeys(void);

    /** Access the this pointer. */
    static GTOP_Display* GetThis(void) {return fGTop_Display;};


private:
    void start_display(void);
    void end_display(void);
    void display_message (const char *fmt, ...);

    /* Position screen stuff. */
    void display_position(double lat, double lon, double alt, 
			  double geoid, float time, uint8_t fix);
    void display_velocity( float ctrue, float cmag, float speedN, float speedK);
    void display_mode(unsigned char mode);
    void display_rp(unsigned char manual_mode, unsigned char nsvs, 
		    unsigned char ndim, const unsigned char *sv_prn,
		    float pdop, float hdop, float vdop, float tdop);
    void display_time(time_t gpstime, double delta);
    
    /* Mainpulate command area. */
    void DisplayCommandChar(unsigned char c);
    void ClearCommandArea(void);

    WINDOW *fVin = NULL;
    int fCurrentScreen;
    bool fRun;
    bool fDisplayData; 

    // Store this pointer. 
    static GTOP_Display* fGTop_Display;
};

/*
 * Thread for maintaining the display.
 */
void* DisplayThread(void *arg);

#endif

