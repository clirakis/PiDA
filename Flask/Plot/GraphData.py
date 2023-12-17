"""@GraphData.py
   Persistantly manage X-Y data for use on a web app. 


   Modified  By   Reason
   --------  --   ------
   16-Dec-23 CBL  Original
   
   References:
   -----------
   
"""
import numpy as np

class GraphData(object):
    def __init__(self, NPoints=1000):
        """@brief This will create a new structure that will support
        persistantly storing X & Y data
        """
        self.__nbins       = NPoints   # upper size of vectors
        self.__xbins       = np.zeros(1)
        self.__ybins       = np.zeros(1)
        self.__NPoints     = 0
        self.__Error       = False
        self.__CodeVersion = 1.0
        self.debug         = False
        print(' GRAPH DATA INIT')

    def clear(self):
        """@clear
        Usually called when we change from LL to XY.
        The data in the arrays are in one format or another
        and need to be flushed. 
        """
        self.__xbins   = np.zeros(1)
        self.__ybins   = np.zeros(1)
        self.__NPoints = 0

    def AddPoint(self,x,y):
        """@AddPoint - add XY point to set
        @x - new x point
        @y - new y point
        """
        if (self.__NPoints+1 > self.__nbins):
            # Shift current array left and add points
            # May want to block this depending on how slow it is. 
            self.__xbins = np.roll(self.__xbins, -1)
            self.__ybins = np.roll(self.__ybins, -1)
            self.__xbins[self.__NPoints] = x
            self.__ybins[self.__NPoints] = y
        else:
            self.__xbins = np.append(self.__xbins, x)
            self.__ybins = np.append(self.__ybins, y)
            self.__NPoints = self.__NPoints + 1

    def X(self):
        """
        @brief Access X data.
        """
        return self.__xbins

    def Y(self):
        """
        @brief Access X data.
        """
        return self.__ybins

    
    def Mean(self):
        """
        @brief return the mean of the x and y 
        """
        self.__Error = False
        return np.mean(self.__xbins),np.mean(self.__ybins)
        
        
    def Std(self):
        """
        @brief return the std of the x and y 
        """
        self.__Error = False
        return np.std(self.__xbins),np.std(self.__ybins)
        

    def IsError(self):
        """
        @brief returns true if an error occured during the last call.
        """
        return self.__Error

    def Version(self):
        return self.__CodeVersion

    def __str__(self):
        """
        Dump data about this structure. 
        """
        rep  = "Graph Data --------------------------------- " + "\n"
        rep += "    NPoints: " + str(self.__NPoints) + "\n"
        xn,yn = self.Mean()
        rep += "     Mean X: " + str(xn) + " Y: " + str(yn) + "\n"
        xn,yn = self.Std()
        rep += "      Std X: " + str(xn) + " Y: " + str(yn) + "\n"
        rep += " -------------------------------------------  \n"
        return rep
