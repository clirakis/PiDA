#!/usr/bin/python3
# 09-Oct-22
# do not run this an an executable. I installed all these pieces
# in a venv.
#
# pygeodesy as part of PyPI builds on top of PyProj with a whole lot
# of utlities.
#
# I did verify forward and reverse 
#
#############################################################
import matplotlib.pyplot as plt
import numpy as np
import time
from pygeodesy import utm
from pygeodesy import points
from pygeodesy.ellipsoidalVincenty import LatLon

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
    #
    # So, both of these methods for making lat/lon points works. 
    #p = points.LatLon_(lat=Lat,lon=Lon)
    p = LatLon(Lat, Lon)
    print("Points: ", p)
    #
    u = utm.toUtm8(Lat, Lon)
    print("UTM: ", u)
    print("X: " , u.easting, " Y:", u.northing, " DX: ", u.easting-X0, " DY: ", u.northing-Y0)

    #
    # https://gis.stackexchange.com/questions/241105/how-to-compute-latitude-and-longitude-of-a-point-at-a-distance-from-another-poin

    # Use our position and see where we land if I use the range bearing given
    # from pyproj
    Range   = 217858.76437249876  # meters
    Bearing = 83.54               # degrees
    # https://mrjean1.github.io/PyGeodesy/
    # There are a bunch of functions associated with LatLon
    #
    d = p.destination(Range, Bearing)
    print("New destitation: ", d)

    
    #
    # AWESOME
    # More on this
    # https://community.esri.com/t5/coordinate-reference-systems-blog/distance-on-an-ellipsoid-vincenty-s-formulae/ba-p/902053
    #
    
    # and back.
    #LY, LX = pGeo.ToLL(X,Y)
    #print("Lat:", LY, " Lon:", LX)

    LatRI = 41.5
    LonRI = -71.3
    #az,R = pGeo.RangeBearing(Lon, Lat, LonRI, LatRI)
    #print("Azimuth: ", az," Range(m):", R)
