"""@Geodetic
Utilities to interface to proj and help do projections back and forth.

     Modified  By   Reason
     --------  --   ------
     10-Feb-22 CBL  Original

References:
https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_geodetic_to_ECEF_coordinates 
https://pyproj4.github.io/pyproj/stable/ 
https://pyproj4.github.io/pyproj/stable/examples.html 
https://proj.org/faq.html#what-is-the-best-format-for-describing-coordinate-reference-systems

Place to validate projections
https://www.earthpoint.us/convert.aspx
https://scienceweb.whoi.edu/marine/ndsf/cgi-bin/NDSFutility.cgi?form=0&from=LatLon&to=UTM
"""
import numpy as np
from pyproj import Proj
#from pyproj import CRS This is in a newer version. I am stuck at 4
import pyproj
from pyproj import Geod

class Geodetic(object):
    def __init__(self):
        self.__Latitude  = 0.0  # degrees latitude for center of projection
        self.__Longitude = 0.0  # degrees longitude for center of projection
        self.__X0        = 0.0
        self.__Y0        = 0.0
        self.__Zone      = 0
        self.SetCenter(41.3, -73.89)

    def CalculateZone(self, Longitude):
        """
        @Longitude in degress
        returns Zone as integer
        """
        Zone = Longitude + 180.0
        Zone = np.ceil(Zone/6.0)
        return Zone

    def SetCenter(self, Latitude, Longitude):
        """
        SetCenter  - reset the center for the projection
        @Latitude  - degrees latitude for center of projection
        @Longitude - degrees longitude for center of projection
        """
        self.__Latitude  = Latitude
        self.__Longitude = Longitude
        self.__Zone = self.CalculateZone(self.__Longitude)
        self.prj = Proj(proj='utm',zone =self.__Zone, ellps='WGS84')
        self.geod = Geod(ellps='WGS84')
        #print(self.prj.proj_version)
        # initialize projection
        # ECEF stuff
        #ecef = Proj(proj='geocent', ellps='WGS84', datum='WGS84')
        #self.prj = Proj(proj='latlong', ellps='WGS84', datum='WGS84')

        # ordinary surface projections
        self.__X0, self.__Y0 = self.prj(self.__Longitude, self.__Latitude)
        #

    def ToXY(self, Lon, Lat):
        """
        Lon in degrees
        Lat in degrees
        return X,Y in UTM
        """
        X,Y = self.prj(Lon, Lat)
        return X,Y

    def ToLL(self, X, Y):
        """
        X - East in meters
        Y - North in meters
        returns Lat, Lon
        """
        Lon, Lat = self.prj(X,Y,inverse=True)
        return Lat, Lon

    
    def RangeBearing(self, Lon0, Lat0, Lon1, Lat1):
        """
        Lon0 - point 0 longitude in degrees
        Lat0 - point 0 latitude in degrees
        Lon1 - point 1 ditto
        Lat1 - point 1 ditto
        returns azimuth in both directions and range
        """
        az12,az21,dist = self.geod.inv(Lon0, Lat0, Lon1, Lat1)
        return az12,dist
        
    def __str__(self):
        rep = 'Projection data. Proj version:'+str(self.prj.proj_version)+'\n'
        rep += 'Center Latitude: ' + str(self.__Latitude)
        rep += ', Longitude:' + str(self.__Longitude)
        rep += ', Zone: ' + str(self.__Zone)
        rep += '\n'
        rep += 'X0:' + str(self.__X0) + ', Y0:' + str(self.__Y0)
        rep += '\n'
        return rep
