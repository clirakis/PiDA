"""@PositionPlot.py
   This started with the following code:
       https://matplotlib.org/3.3.1/gallery/lines_bars_and_markers/scatter_hist.html#sphx-glr-gallery-lines-bars-and-markers-scatter-hist-py

   It has been slowly modified to support plotting the postition information
   from a GPS system to look at how the X-Y of the system evolves.

   Modified  By   Reason
   --------  --   ------
   22-Nov-20 CBL  Original
   23-Nov-20 CBL  Changed the central plot from scatter to plot
                  for faster update.
   23-Nov-20 CBL  version 3, update point pair at a time.
                  fixed a bunch of bugs, added methods for mean and std.
                  Also, put in a error flag.
                  version 4 make variables hidden with 2 underbars.
                  version 0.5 now a class
   25-Nov-20 CBL  Adding in legends with some data.
   05-Feb-22 CBL  incorporating into FlaskDA
                  for some reason the update method is slow inside the server
                  on the order of 3 to 3.5 seconds per update.
                  Modify the update method to do something like is shown
                  here:
                  https://stackoverflow.com/questions/53630158/add-points-to-the-existing-matplotlib-scatter-plot

   06-Feb-22 CBL  Modifying how we create and present plots.
   12-Nov-22 CBL  Clean up on documentation. Issues with scaling. 

   
   References:
   -----------
   https://stackoverflow.com/questions/29277080/efficient-matplotlib-redrawing
   https://matplotlib.org/3.3.1/gallery/animation/strip_chart.html#sphx-glr-gallery-animation-strip-chart-py
   
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
import io

class PositionPlot(object):
    def __init__(self):
        """@brief This will create a new structure that will support
        a scatter plot and the projections along the X & Y axes.
        This will also create the figure. 
        """

        # Histogram and scatterplot definition bin. Upper and lower Y
        # only make sense for the scatter plot. 
        self.__nbins       = 80
        self.__LowerX      = -1.0
        self.__UpperX      =  1.0
        self.__LowerY      = -1.0
        self.__UpperY      =  1.0
        
        self.__ScatterPlot = None
        self.__xbins       = None
        self.__ybins       = None
        self.__xhist       = None
        self.__yhist       = None
        self.__Error       = False
        self.__CodeVersion = 1.0

        self.debug         = False
        
        #
        # define the layout of the scatterplot and histograms.
        #
        # definitions for the axes, This lays out the boundaries
        #
        left, width    = 0.1, 0.65
        bottom, height = 0.1, 0.65
        spacing        = 0.005
        rect_scatter   = [left, bottom, width, height]
        rect_histx     = [left, bottom + height + spacing, width, 0.2]
        rect_histy     = [left + width + spacing, bottom, 0.2, height]
        
        # start with a square Figure: 8x8 inches
        #  https://matplotlib.org/api/_as_gen/matplotlib.figure.Figure.html
        # one central figure. Really all the data is held in the
        # axis so we don't need to strore this. 
        self.__fig = plt.figure(figsize=(8, 8))

        #
        # create the central figure for the scatter plot
        #
        self.__ax = self.__fig.add_axes(rect_scatter)
        self.__ax.grid(True)
        #self.__ax.set_title('GPS X-Y')
        #self.__ax.set_xlabel('Centered False Easting')
        #self.__ax.set_ylabel('Centered False Northing')
        self.__ax.set_xlabel('Latitude')
        self.__ax.set_ylabel('Longitude')
        self.__ax.set_xlim(self.__LowerX, self.__UpperX)
        self.__ax.set_ylim(self.__LowerY, self.__UpperY)

        #
        # create a histogram for the x projection
        #
        self.__ax_histx = self.__fig.add_axes(rect_histx, sharex=self.__ax)
        self.__ax_histx.grid(True)

        #
        # create a histogram for the y projection
        #
        self.__ax_histy = self.__fig.add_axes(rect_histy, sharey=self.__ax)
        self.__ax_histy.grid(True)
        print("Position Plot Constructor.")

    def setXLabel(self, label):
        self.__ax.set_xlabel(label)

    def setYLabel(self, label):
        self.__ax.set_ylabel(label)

        
    def SetXLimits(self, lowerX, upperX):
        """@brief Set the X lower and upper limits on the plots.
        @lowerX - lower X bin value for scatter plot
        @upperX - upper X bin value for scatter plot
        """
        self.__LowerX       = lowerX
        self.__UpperX       = upperX
        # Set for central plot
        self.__ax.set_xlim(self.__LowerX, self.__UpperX)
        self.__ax.set_ylim(self.__LowerY, self.__UpperY)
        binwidth = (upperX - lowerX)/self.__nbins
        #
        # This depends a bit on if we are - or +
        #
        if (lowerX < upperX):
            self.__xbins = np.arange(lowerX, upperX, binwidth)
        else:
            self.__xbins = np.arange(upperX, lowerX, binwidth)

        #
        # Make our own bin labels
        # Example for putting a comma in an existing label set.
        # current_values = plt.gca().get_yticks()
        # plt.gca().set_yticklabels(['{:,.0f}'.format(x) for x in current_values])
        # IN our case fix the number of labels
        #
        dX = (upperX-lowerX)/4.0
        #labels = np.around(np.arange(lowerX, upperX, dX), decimals=3)
        labels = np.arange(lowerX, upperX, dX)
        self.__ax.set_xticks(labels)

        # set the format on the axis
        #self.__ax.set_xticklabels(['3.5f'])
        # string format example
        # precision = 4
        # print( "{:.{}f}".format( pi, precision ) )
        first = "{:.{}f}".format( labels[0], 7 )
        fpart, ipart = np.modf(labels)
        second = "{:.{}f}".format( fpart[1], 7 )
        third  = "{:.{}f}".format( fpart[2], 7 )
        fourth = "{:.{}f}".format( fpart[3], 7 )
        
        fmtlabel = [first, second, third, fourth]
        self.__ax.set_xticklabels(fmtlabel)

    def SetYLimits(self, lowerY, upperY):
        """@brief Set the X and Y limits the scatterplot.
        """
        self.__LowerY       = lowerY
        self.__UpperY       = upperY
        # set for central plot
        self.__ax.set_xlim(self.__LowerX, self.__UpperX)
        self.__ax.set_ylim(self.__LowerY, self.__UpperY)
        binwidth = (upperY - lowerY)/self.__nbins
        self.__ybins = np.arange( lowerY, upperY, binwidth)

        dY = (upperY-lowerY)/4.0
        labels = np.arange(lowerY, upperY, dY)
        #print("Y labels:", labels)
        self.__ax.set_yticks(labels)
        
        #self.__ax.locator_params(axis='y', nbins = 4)

    def clear(self):
        """@clear
        Usually called when we change from LL to XY.
        The data in the arrays are in one format or another
        and need to be flushed. 
        """
        x = []
        y = []
        self.__ScatterPlot.set_xdata(x)
        self.__ScatterPlot.set_ydata(y)
        self.__xhist = self.__ax_histx.hist(x, bins=self.__xbins,color='b')
        self.__yhist = self.__ax_histy.hist(y, bins=self.__ybins, orientation='horizontal', color='b')

    def FillHist(self,x,y):
        """@FillHist - fill the histograms with the new values
        @x - new x point
        @y - new y point
        """
        self.__xhist = self.__ax_histx.hist(x, bins=self.__xbins,color='b')
        self.__yhist = self.__ax_histy.hist(y, bins=self.__ybins,
                                            orientation='horizontal',
                                            color='b')

    def BlockFill(self, x, y):
        """@brief Method to update the current points in the plot.
        This entry point has all the points.
        NOTE: The self.x and self.y are not updated. 
        @param x - vector of x points to update
        @param y - vector of y points to update
        """
        self.__Error = False

        # no labels
        self.__ax_histx.tick_params(axis="x", labelbottom=False)
        self.__ax_histy.tick_params(axis="y", labelleft=False)

        #
        # the scatter plot, using the plot command
        # since I won't use multiple symbols/markers and this
        # is substantially faster.
        #
        self.__ScatterPlot, = self.__ax.plot(x, y, 'g.')

        # now determine nice limits by hand, kind of autoscaling
##        xymax = max(np.max(np.abs(x)), np.max(np.abs(y)))
##        lim = (int(xymax/self.__binwidth) + 1) * self.__binwidth
##        bins = np.arange(-lim, lim + self.__binwidth, self.__binwidth)
        self.__xhist = self.__ax_histx.hist(x, bins=self.__xbins,color='b')
        self.__yhist = self.__ax_histy.hist(y, bins=self.__ybins, orientation='horizontal', color='b')
        
    def Mean(self):
        """
        @brief return the mean of the x and y axis based on the
        points currently in the scatter plot.
        """
        self.__Error = False
        if (self.__ScatterPlot):
            # A plot does exist to extract data from. 
            x,y = self.__ScatterPlot.get_data()
            return np.mean(x),np.mean(y)
        else:
            self.__Error = True
            return -9999999, -9999999
        
    def Std(self):
        """
        @brief return the std of the x and y axis based on the
        points currently in the scatter plot.
        """
        self.__Error = False
        if (self.__ScatterPlot == None):
            self.__Error = True
            return -9999999, -9999999
        else:
            # A plot does exist to extract data from. 
            x,y = self.__ScatterPlot.get_data()
            return np.std(x),np.std(y)
        
    def addPoint(self, xp, yp):
        """@brief Method to update the current points in the plot.
        the previous instantiation updated a single block at a time
        rather than using a point pair.
        @param xp - point(s) to update
        @param yp - point(s) to update
        """
        self.__Error = False

        # This deals with the central plot well. 
        if (self.__ScatterPlot == None):
            # plot returns a list, putting the comma here
            # returns just the first entry in the list. 
            self.__ScatterPlot, = self.__ax.plot(xp, yp, 'g.')
        else:
            # get the data from the source and add to it. 
            x,y = self.__ScatterPlot.get_data(orig=True)

            if ((x!= 0) and (y!=0)):
                if(self.debug):
                    print("Position Plot add point.", x, " ", y)
                np.append(x, xp)
                np.append(y, yp)
                self.__ScatterPlot.set_xdata(x)
                self.__ScatterPlot.set_ydata(y)
                self.__ax.plot(xp, yp, 'g.')

                # update histograms.
                self.FillHist(xp,yp)

    def draw(self):
        """
        @brief update the plot on the screen.
        """
        self.__ax.figure.canvas.draw()
        self.addStatistics()
        self.__ax.figure.canvas.flush_events()
        plt.pause(0.001)
        
    def refresh(self):
        """
        @brief do a full refresh of the drawing.
        """
        # This does not appear to work. 
        self.__ax.figure.canvas.draw()
        plt.show(block=False)
        #self.__ax.figure.canvas.flush_events()

    def addStatistics(self):
        """
        @brief Get the statistics about the plot and put them in a
        text box that the user can read the basic numbers.
        """
        muX,muY = self.Mean()
        sigmaX,sigmaY = self.Std()
        str1 = ' '.join((r'$\mu_X=%.2f$' % (muX, ),
                        r'$\sigma_X=%.2f$' % (sigmaX, )))
        str2 = ' '.join((r'$\mu_Y=%.2f$' % (muY, ),
                        r'$\sigma_Y=%.2f$' % (sigmaY, )))

        textstr = '\n'.join((str1, str2))
##        textstr = '\n'.join((
##            r'$\mu_X=%.2f$' % (muX, ), r'$\sigma_X=%.2f$' % (sigmaX, ),
##            r'$\mu_Y=%.2f$' % (muY, ),
##            r'$\sigma_Y=%.2f$' % (sigmaY, )))

        # these are matplotlib.patch.Patch properties
        props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)

        # place a text box in upper left in axes coords
        self.__ax.text(0.05, 0.95, textstr,
                       transform=self.__ax.transAxes, fontsize=14,
                       verticalalignment='top', bbox=props)

    def IsError(self):
        """
        @brief returns true if an error occured during the last call.
        """
        return self.__Error

    def show(self):
        """
        Usual matplotlib show for the plot.
        """
        plt.show()

    def Version(self):
        return self.__CodeVersion

    def InlinePlot(self):
        canvas = FigureCanvas(self.__fig)
        output = io.BytesIO()
        canvas.print_png(output)
        return output
