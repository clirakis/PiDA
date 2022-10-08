#!/usr/bin/python3
import matplotlib.pyplot as plt
import numpy as np
import time

from Geodetic import Geodetic as Geo

if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    pGeo = Geo()
    print(pGeo)
    # https://www.earthpoint.us/convert.aspx
    # results for default center should be
    # 18T 590033mE 4794728mN
    #
    # https://scienceweb.whoi.edu/marine/ndsf/cgi-bin/NDSFutility.cgi?form=0&from=LatLon&to=UTM
    # 592928.9   4572654.9   Zone 18 CHECK
    # Google Maps position for 217 Locust Ave
    Lat = 41.308489
    Lon = -73.893027
    X,Y = pGeo.ToXY(Lon, Lat)
    print("X: " , X, " Y:", Y)

    # and back.
    LY, LX = pGeo.ToLL(X,Y)
    print("Lat:", LY, " Lon:", LX)

    LatRI = 41.5
    LonRI = -71.3
    az,R = pGeo.RangeBearing(Lon, Lat, LonRI, LatRI)
    print("Azimuth: ", az," Range(m):", R)
