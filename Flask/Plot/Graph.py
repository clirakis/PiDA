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
        self.__lat_min__ = 41.0
        self.__lat_max__ = 42.0
        self.__lon_min__ = -72.0
        self.__lon_max__ = -71.0

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
