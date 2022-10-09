"""@ProjectedPlot.py

   Build on top of PositionPlot class, but used projected x/y I.E.
   Thread in geodetic data. 

   Modified  By   Reason
   --------  --   ------
   09-Oct-22 CBL  Original
   
"""
import numpy as np
from PositionPlot import PositionPlot
from pygeodesy.ellipsoidalVincenty import LatLon


class ProjectedPlot(PositionPlot):
    def __init__(self):
        """@brief using geodetic projections we can create x-y plots.
        no input now
        """
        super().__init__(self)
        self.__scale = 1000      # initial scale size in meters

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

        
