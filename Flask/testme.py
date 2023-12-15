#!/usr/bin/python3
import numpy as np
import time

from PySM.NMEA_GGA import NMEA_GGA
from PySM.NMEA_RMC import NMEA_RMC
from PySM.NMEA_GSA import NMEA_GSA
from PySM.NMEA_VTG import NMEA_VTG


# put my initialization in here. This one attaches to the
# time position shared memory if it exists.
# This is based on POSIX shared memory and is tightly linked
# to a binary data acquisition module I have called lassen. 
#MySM = NMEA_GGA()
MySM = NMEA_GSA()
MySM.debug = True

#
SleepTime = 1 # seconds between signals.
global t0
t0 = time.time()


def getGPS_Position():
    """
    This interfaces to the shared memory of the lassen GPS using POSIX
    shared memory and semaphores. This is all managed by PySharedMem2.py
    return the
    time of fix in gps seconds
    time of fix read in seconds since epoch
    latitude in radians
    longitude in radians
    altitude in meters from the mean geoid
    """
    # Initialize variables
    gps_time = 0
    lat = 0.0
    lon = 0.0
    z = 0.0
    
    if (MySM.NoError()):
        MySM.Read()
        print(MySM)
        gps_time = MySM.fSec  # This isn't really the time
        fix_time = MySM.LastRead_Time_tv_sec
        lat  = MySM.fLatitude
        lon  = MySM.fLongitude
        z    = MySM.fAltitude
    else:
        print('Error in reading from SM.')
        gps_time = 0
        fix_time = time.time()
        lat = np.deg2rad(41.5)
        lon = np.deg2rad(-71.2)
        z = 0.0
    return gps_time, fix_time, lat, lon, z

def SignalHandler(signum, frame):
    """
    Wake up and check for new GPS data for the plots. 
    """
    global t0
    if (signum == signal.SIGALRM) :
        t1 = time.time()
        dt = t1-t0
        t0 = t1
        gps_t, fix_time, Lat, Lon, z = getGPS_Position()
        print("Timeout: ", dt)
        x = np.rad2deg(Lon)
        y = np.rad2deg(Lat)
        # make it happen again. 
        signal.alarm(1)

if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """

#    MySMName.Read()
#    MySMName.Print()
    
    
    for i in range(5):
        #gps_t, fix_time, Lat, Lon, z = getGPS_Position()
        # Test GSA
        MySM.Read()
        print(MySM)
        t1 = time.time()
        dt = t1-t0
        t0 = t1
        print("Timeout: ", dt)
        time.sleep(1)
    time.sleep(5)
