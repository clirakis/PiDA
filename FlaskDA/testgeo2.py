#!/usr/bin/python3
import matplotlib.pyplot as plt
import numpy as np
import time
from pygeodesy import utm
from pygeodesy import points

if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    # https://www.earthpoint.us/convert.aspx
    # results for default center should be
    # 18T 590033mE 4794728mN
    #
    # https://scienceweb.whoi.edu/marine/ndsf/cgi-bin/NDSFutility.cgi?form=0&from=LatLon&to=UTM
    # 592648.4 4573593.9   Zone 18 CHECK
    #
    # 08-Oct-22
    X0 =  592648.4
    Y0 = 4573593.9
    # Google Maps position for 217 Locust Ave
    Lat =  41.308489
    Lon = -73.893027
    print('UTM ', utm)
    print('Zone: ', utm.utmZoneBand5(Lat, Lon))
    p = points.LatLon_(lat=Lat,lon=Lon)
    print("Points: ", p)
    u = utm.toUtm8(Lat, Lon)
    print("UTM: ", u)
    print("X: " , u.easting, " Y:", u.northing, " DX: ", u.easting-X0, " DY: ", u.northing-Y0)

    # and back.
    #LY, LX = pGeo.ToLL(X,Y)
    #print("Lat:", LY, " Lon:", LX)

    LatRI = 41.5
    LonRI = -71.3
    #az,R = pGeo.RangeBearing(Lon, Lat, LonRI, LatRI)
    #print("Azimuth: ", az," Range(m):", R)
