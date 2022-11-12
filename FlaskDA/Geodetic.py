"""@Geodetic
Utilities to interface to proj and help do projections back and forth.

     Modified  By   Reason
     --------  --   ------
     10-Feb-22 CBL  Original
     07-Nov-22 CBL  does not work.
     11-Nov-22 CBL  update with pyproj

References:
https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_geodetic_to_ECEF_coordinates 
https://pyproj4.github.io/pyproj/stable/ 
https://pyproj4.github.io/pyproj/stable/examples.html 
https://proj.org/faq.html#what-is-the-best-format-for-describing-coordinate-reference-systems

Place to validate projections
https://www.earthpoint.us/convert.aspx
https://scienceweb.whoi.edu/marine/ndsf/cgi-bin/NDSFutility.cgi?form=0&from=LatLon&to=UTM

https://pyproj4.github.io/pyproj/stable/examples.html
This suports proj 8.2 and above. 
"""
import numpy as np
from pyproj import CRS
from pyproj import Transformer
from pyproj import Geod


class Geodetic(object):
    def __init__(self):
        self.__Latitude  = 0.0  # degrees latitude for center of projection
        self.__Longitude = 0.0  # degrees longitude for center of projection
        self.__X0        = 0.0
        self.__Y0        = 0.0
        self.__Zone      = 0
        self.setCenter(41.3, -73.89)
        

    def calculateZone(self, Longitude):
        """
        @Longitude in degress
        returns Zone as integer
        """
        Zone = Longitude + 180.0
        Zone = np.ceil(Zone/6.0)
        return Zone

    def setCenter(self, Latitude, Longitude):
        """
        SetCenter  - reset the center for the projection
        @Latitude  - degrees latitude for center of projection
        @Longitude - degrees longitude for center of projection
        """
        self.__Latitude  = Latitude
        self.__Longitude = Longitude
        self.__Zone = self.calculateZone(self.__Longitude)

        crs_4326  = CRS.from_epsg(4326) # WGS 84
        # 32618 - UTM zone 18
        crs_in    = 32600 + self.__Zone
        crs_32618 = CRS.from_epsg(crs_in)
        # Forward to UTM XY
        self.__fwd = Transformer.from_crs(crs_4326, crs_32618)
        # go in reverse.
        self.__inv = Transformer.from_crs( crs_32618, crs_4326)
        self.__X0, self.__Y0 = self.__fwd.transform(self.__Longitude, self.__Latitude)
        #

    def centerLL(self):
        return self.__Latitude, self.__Longitude
    
    def centerXY(self):
        return self.__X0, self.__Y0

    def ToXY(self, Lon, Lat):
        """
        Lon in degrees
        Lat in degrees
        return X,Y in UTM
        """
        X,Y = self.__fwd.transform(Lon, Lat)
        return X,Y

    def ToLL(self, X, Y):
        """
        X - East in meters
        Y - North in meters
        returns Lat, Lon
        Look at this for using the Transformer method
        https://gis.stackexchange.com/questions/78838/converting-projected-coordinates-to-lat-lon-using-python
        """
        Lat, Lon = self.__inv.transform(X,Y)
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
