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
        #
        # Create the projection and set the center of the projection
        # with the supplied coordinates. 
        self.__geo        = Geodetic()
        self.__geo.setCenter(Lat, Lon)
        
        self.__scale      = 1000      # initial scale size in meters

        # Set the display center, this is not necessiarly the
        # same as the projection center, but stat that way. 
        self.__center_Lat =  41.0
        self.__center_Lon = -71.5
        self.__X0 = 0
        self.__Y0 = 0
        self.Center(Lat, Lon)

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
        self.__center_Lat = Lat
        self.__center_Lon = Lon
        self.calculateLimits()

    def calculateLimits(self):
        """@brief calculate the four corners of a polygon for display
        given the center latitude and longitude
        @Lat - lattitude in degrees of center of polygon
        @Lon - longitude in degrees of the center of polygon
        @scale - scale value in meters side to side.
        """
        print("Points: ", self.__center_Lat, " ", self.__center_Lon)

        self.__X0, self.__Y0 = self.__geo.ToXY(self.__center_Lat, self.__center_Lon)
        print("X: " , self.__X0, " Y:", self.__Y0)

        XL = self.__X0 - self.__scale/2
        XR = self.__X0 + self.__scale/2
        YL = self.__Y0 - self.__scale/2
        YR = self.__Y0 + self.__scale/2

        print("Upper Right: ", XR, " ", YR)
        print("Lower Left:  ", XL, " ", YL)


        ur_Lat,ur_Lon = self.__geo.ToLL(XR,YR)
        ll_Lat,ll_Lon = self.__geo.ToLL(XL,YL)
    
        # Calculate Left edge - assume north up
        print("Upper Right: ", ur_Lat, " " , ur_Lon)
        print("Lower Left:  ", ll_Lat, " ", ll_Lon)

        self.SetXLimits(ll_Lon, ur_Lon)
        self.SetYLimits(ll_Lat, ur_Lat)
