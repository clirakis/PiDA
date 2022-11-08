"""@ProjectedPlot.py

   Build on top of PositionPlot class, but used projected x/y I.E.
   Thread in geodetic data. 

   Modified  By   Reason
   --------  --   ------
   09-Oct-22 CBL  Original
   07-Nov-22 CBL  Added in calculate center

   References:
   pygeodesy as part of PyPI builds on top of PyProj with a whole lot
   of utlities.
   https://pypi.org/project/PyGeodesy/
   https://mrjean1.github.io/PyGeodesy/
   http://docs.ros.org/en/kinetic/api/geodesy/html/python/index.html

"""
import numpy as np
from Plotting.PositionPlot import PositionPlot
from pygeodesy.ellipsoidalVincenty import LatLon
from pygeodesy import utm
from pygeodesy import points


class ProjectedPlot(PositionPlot):
    def __init__(self):
        """@brief using geodetic projections we can create x-y plots.
        no input now
        """
        PositionPlot.__init__(self)
        self.__scale  = 1000      # initial scale size in meters
        self.__center = LatLon(41.0, -71.5)
        self.calculateLimits()
        

    def Scale(self):
        """@brief method for retrieving the scale in meters.
        No input
        """
        return self.__scale

    def Scale(self, val):
        """@brief method to set new scale value
        @val input in meters
        """
        self.__scale = val
        self.calculateLimits()

    def Center(self):
        """@brief Get the current display center.
        """
        return self.__center
    
    def Center(self, Lat, Lon):
        """@brief Set the current display center.
        """
        self.__center = LatLon(Lat,Lon)
        self.calculateLimits()

    def calculateLimits(self):
        """@brief calculate the four corners of a polygon for display
        given the center latitude and longitude
        @Lat - lattitude in degrees of center of polygon
        @Lon - longitude in degrees of the center of polygon
        @scale - scale value in meters side to side.
        """
        print("Points: ", self.__center)
        
        # calculate the window based on the four corners, the range is
        # on a 45 degree angle which is scale/(2 root(2))
        Range   = self.__scale/(2.0*np.sqrt(2.0))
        Bearing = 45.0

        # Calculate Left edge - assume north up
        ur = self.__center.destination(Range, Bearing)
        print("Upper Right: ", ur)
        
        Bearing = 135.0
        ll = self.__center.destination(Range, Bearing)
        print("Lower Left: ", ll)

        self.SetXLimits(ll.lon, ur.lon)
        self.SetYLimits(ll.lat, ur.lat)
