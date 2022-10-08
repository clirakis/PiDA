#!/usr/bin/python3
import matplotlib.pyplot as plt
import numpy as np
import time

from PySM.TSIPosition import TSIPosition
from PySM.GPSFilename import GPSFilename
from Plotting.PositionPlot import PositionPlot
from Geodetic import Geodetic as Geo

# Global variable
#
# Plotting tools.
#
PPlot = PositionPlot()

# put my initialization in here. This one attaches to the
# time position shared memory if it exists.
# This is based on POSIX shared memory and is tightly linked
# to a binary data acquisition module I have called lassen. 
MySM = TSIPosition()

MySMName = GPSFilename()

pGeo = Geo()

#
SleepTime = 1 # seconds between signals.
global t0
t0 = time.time()

global X0, Y0

def SetLimits(Lat0, Lon0, Scale):
    """
    Determine the scale based on the center given by
    Lat0 - Center latitude of the plot in degrees
    Lon0 - Center longitude of the plot in degrees
    Scale - in meters from left to right
    """
    # utm center of plot
    global X0, Y0
    Lat = 41.308489
    Lon = -73.893027
    X0,Y0 = pGeo.ToXY(Lon, Lat)

    XH = X0 + Scale/2 
    XL = X0 - Scale/2
    YH = Y0 + Scale/2
    YL = Y0 - Scale/2

    # and back to Lat/Lon Lower Left
    LL_Lat, LL_Lon = pGeo.ToLL(XL,YL)

    # Upper right
    UR_Lat, UR_Lon = pGeo.ToLL(XH, YH)

    print('X low:', LL_Lon, ' X high:', UR_Lon)
    print('Y low:', LL_Lat, ' Y high:', UR_Lat)
    
    PPlot.SetXLimits(LL_Lon, UR_Lon)
    PPlot.SetYLimits(LL_Lat, UR_Lat)


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
        PPlot.update(x,y)
        # make it happen again. 
        signal.alarm(1)

if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """

    MySMName.Read()
    MySMName.Print()
    
    #signal.signal(signal.SIGALRM, SignalHandler)
    #signal.alarm(1)
##    PPlot.SetXLimits(-73.894, -73.890)
##    PPlot.SetYLimits(41.3083, 41.3085)

    Lat0 = 41.308489
    Lon0 = -73.893027
    SetLimits(Lat0, Lon0, 100.0)
    
    for i in range(10):
        gps_t, fix_time, Lat, Lon, z = getGPS_Position()
        t1 = time.time()
        dt = t1-t0
        t0 = t1
        print("Timeout: ", dt)
        x = np.rad2deg(Lon)
        y = np.rad2deg(Lat)
        #print('X:', x, ' Y: ', y)
        PPlot.AddPoint(x,y)
        PPlot.draw()   # This costs a lot!!!

        time.sleep(1)
    time.sleep(5)
