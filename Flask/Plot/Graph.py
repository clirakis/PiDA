"""@package Graph.py - simple graphing
--! @brief Simple plotting of a graph within the Flask environment.

Modifications:
17-Dec-23  CBL  Original

References:


"""
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
import io

from Plot.GraphData import GraphData

class Graph(GraphData):
    """
    Starting over with a simple graph using matplotlib. May migrate to
    plotly or Dash. 
    """
    def __init__(self):
        """
        Constructor
        Nothing big yet.
        """
        GraphData.__init__(self)
        self.__Running = True

        # create a figure
        self.__fig, self.__ax = plt.subplots()
        self.__ax.grid(True)
        self.__ax.set_ylabel('Latitude')
        self.__ax.set_xlabel('Longitude')
        self.__ax.set_title('Simple Plot')

        self.__scale__ = 500.0   # meters
        self.setCenter(41.487, -71.274)


    def setCenter(lat, lon):
        self.__lat0__    = lat
        self.__lon0__    = lon
        self.setLimits(self.__scale__)

    def setLimits(self, scale):
        """
        @param scale - scale in meters for extents. 
        
        Scale value, deg/meter
        1852 meters per minute of latitude, doesn't work as well for long.
        but eh we will take it.

        NOTE: This only works in the local area!! Not general case!!!
        """
        pm = scale/(1852.0 * 60.0)
        
        self.__lat_min__ = self.__lat0__ - pm
        self.__lat_max__ = self.__lat0__ + pm
        self.__lon_min__ = self.__lon0__ + pm
        self.__lon_max__ = self.__lon0__ - pm

    def __del__(self):
        """
        No cleanup at this time. 
        """
        self.Running = False

    def setXLabel(self, label):
        self.__ax.set_xlabel(label)

    def setYLabel(self, label):
        self.__ax.set_ylabel(label)

    def setTitle(self, title):
        self.__ax.set_title('Simple Plot')

    def InlinePlot(self):
        """
        Output the plot data.
        ready to send to HTML canvas
        """
        plt.plot(self.X(), self.Y(),'.')
        plt.ylim(self.__lat_min__, self.__lat_max__)
        plt.xlim(self.__lon_min__, self.__lon_max__)
        canvas = FigureCanvas(self.__fig)
        output = io.BytesIO()
        canvas.print_png(output)
        return output
