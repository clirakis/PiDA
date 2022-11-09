"""
Test the geo/projection utilities.

Testing after Linux upgrade to 22.2
now at proj 8
pyproj will call tat. 
"""
import matplotlib.pyplot as plt
import numpy as np
import time

from Geodetic import Geodetic as Geo
from pyproj import CRS
from pyproj import Transformer
from pyproj import Geod

def testGeo():
    pGeo = Geo()
    print(pGeo)
    # https://www.earthpoint.us/convert.aspx
    # results for default center should be
    # 18T 590033mE 4794728mN
    #
    # https://scienceweb.whoi.edu/marine/ndsf/cgi-bin/NDSFutility.cgi?form=0&from=LatLon&to=UTM
    # 592648.4 4573593.9    Zone 18 CHECK
    #
    # 08-Oct-22 - fixed this and all is good. 
    X0 =  592648.4
    Y0 = 4573593.9

    # Google Maps position for 217 Locust Ave
    Lat = 41.308489
    Lon = -73.893027
    X,Y = pGeo.ToXY(Lon, Lat)
    print("X: " , X, " Y:", Y, " DX: ", X-X0, " DY: ", Y-Y0)

    # and back.
    LY, LX = pGeo.ToLL(X,Y)
    print("Lat:", LY, " Lon:", LX)

    LatRI = 41.5
    LonRI = -71.3
    az,R = pGeo.RangeBearing(Lon, Lat, LonRI, LatRI)
    print("Azimuth: ", az," Range(m):", R)

def CalculateZone(Longitude):
    """
    @Longitude in degress
    returns Zone as integer
    """
    Zone = Longitude + 180.0
    Zone = np.ceil(Zone/6.0)
    return Zone

def testProj():
    """
    CRS - coordinate reference system
    epsg - European Petroleum Survey Group
    https://pyproj4.github.io/pyproj/stable/api/index.html
    4326 WGS84
    326ZZ - where ZZ is the zone
    """
    X0 =  592648.4
    Y0 = 4573593.9

    # Google Maps position for 217 Locust Ave
    Lat = 41.308489
    Lon = -73.893027
    # LL
    crs_4326 = CRS.from_epsg(4326) # WGS 84
    print(crs_4326)
    crs_4326
    
    # 32618 - UTM zone 18
    crs_in = 32600 + CalculateZone(Lon)
    crs_32618 = CRS.from_epsg(crs_in)
    print(crs_32618)

    # Forward to UTM XY
    transformer = Transformer.from_crs(crs_4326, crs_32618)
    res = transformer.transform(Lat, Lon)
    print(res)
    print("delta: " , res[0]-X0, " ", res[1]-Y0)

    # go in reverse.

    inverse = Transformer.from_crs( crs_32618, crs_4326)
    LL = inverse.transform(res[0], res[1])
    print(LL)

    # And all of this works.

    g = Geod(ellps='WGS84')
    RI_Lat = 41.5
    RI_Lon = -71.9

    az12, az21, dist = g.inv(Lon, Lat, RI_Lon, RI_Lat)

    print("INV: ", az12, " ", az21, " ", dist)

    endlon, endlat, backaz = g.fwd(RI_Lon, RI_Lat, az21, dist)

    print("FWD: " , endlat, " ", endlon, " ", backaz)

    # and this is a bit wonky
    
    
if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    testProj()
