"""@ProjectedPlot.py

   Build on top of PositionPlot class, but used projected x/y I.E.
   Thread in geodetic data. 

   Modified  By   Reason
   --------  --   ------
   09-Oct-22 CBL  Original
   07-Nov-22 CBL  Added in calculate center
   11-Nov-22 CBL  updated calculate limits

   References:
   pygeodesy as part of PyPI builds on top of PyProj with a whole lot
   of utlities.
   https://pypi.org/project/PyGeodesy/
   https://mrjean1.github.io/PyGeodesy/
   http://docs.ros.org/en/kinetic/api/geodesy/html/python/index.html

"""
import numpy as np
from Plotting.PositionPlot import PositionPlot
from Geodetic import Geodetic

class ProjectedPlot(PositionPlot):
    def __init__(self, Lat, Lon):
        """@brief using geodetic projections we can create x-y plots.
        no input now
        @param Lat - projection center
        @param Lon - projection center
        """
        PositionPlot.__init__(self)
        self.__geo        = Geodetic()
        self.__geo.SetCenter(Lat, Lon)
        
        self.__scale      = 1000      # initial scale size in meters

        self.setCenter(Lat, Lon)
        self.__center.Lat = 41.0
        self.__center.Lon = -71.5

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
        self.__center.Lat = Lat
        self.__center.Lon = Lon
        
        self.calculateLimits()

    def calculateLimits(self):
        """@brief calculate the four corners of a polygon for display
        given the center latitude and longitude
        @Lat - lattitude in degrees of center of polygon
        @Lon - longitude in degrees of the center of polygon
        @scale - scale value in meters side to side.
        """
        print("Points: ", self.__center)

        res = fwd.transform(self.__center.Lat, self.__center.Lon)
        self.X0 = res[0]
        self.Y0 = res[1]

        print("X: " , res[0], " Y:", res[1])

        XL = res[0] - scale/2
        XR = res[0] + scale/2
        YL = res[1] - scale/2
        YR = res[1] + scale/2

        print("Upper Right: ", XR, " ", YR)
        print("Lower Left: ", XL, " ", YL)


        ul = inv.transform(XR,YR)
        ll = inv.transform(XL,YL)
    
        # Calculate Left edge - assume north up
        print("Upper Right: ", ul)
        print("Lower Left: ", ll)

        self.SetXLimits(ll[1], ur[1])
        self.SetYLimits(ll[0], ur[0])
