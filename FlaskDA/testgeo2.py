"""
===================================================================
 09-Oct-22
 do not run this an an executable. I installed all these pieces
 in a venv.

 pygeodesy as part of PyPI builds on top of PyProj with a whole lot
 of utlities.

 I did verify forward and reverse

 07-Nov-22 How did I do reverse??

===================================================================
"""
import matplotlib.pyplot as plt
import numpy as np
import time
from pygeodesy import utm
from pygeodesy import points
from pygeodesy.ellipsoidalVincenty import LatLon

def CalculateLimitsTwo(Lat, Lon, scale):
    """@brief calculate the four corners of a polygon for display
    given the center latitude and longitude
    @Lat - lattitude in degrees of center of polygon
    @Lon - longitude in degrees of the center of polygon
    @scale - scale value in meters side to side.
    """
    p = LatLon(Lat, Lon)
    
    u = utm.toUtm8(Lat, Lon)
    print("UTM: ", u)
    print("X: " , u.easting, " Y:", u.northing)

    XL = u.easting - scale/2
    XR = u.easting + scale/2
    YL = u.northing + scale/2
    YR = u.northing - scale/2


    # Calculate Left edge - assume north up
    ul = p.destination(Range, Bearing)
    print("Upper Right: ", ul)

    Bearing = 135.0
    ll = p.destination(Range, Bearing)
    print("Lower Left: ", ll)



def CalculateLimits(Lat, Lon, scale):
    """@brief calculate the four corners of a polygon for display
    given the center latitude and longitude
    @Lat - lattitude in degrees of center of polygon
    @Lon - longitude in degrees of the center of polygon
    @scale - scale value in meters side to side.
    """
    p = LatLon(Lat, Lon)
    print("Points: ", p)

    # calculate the window based on the four corners, the range is
    # on a 45 degree angle which is scale/(2 root(2))
    Range   = scale/(2.0*np.sqrt(2.0))
    Bearing = 45.0

    print("Range: ", Range, " Bearing: ", Bearing);

    # Calculate Left edge - assume north up
    ul = p.destination(Range, Bearing)
    print("Upper Right: ", ul)

    Bearing = 135.0
    ll = p.destination(Range, Bearing)
    print("Lower Left: ", ll)

    Bearing = 0.0
    res = p.destination(Range, Bearing)
    print("Zero: ", res)

    Bearing = 90.0
    res = p.destination(Range, Bearing)
    print("ninety: ", res)
    
    Bearing = 180.0
    res = p.destination(Range, Bearing)
    print("180: ", res)
    
    Bearing = 275.0
    res = p.destination(Range, Bearing)
    print("275: ", res)


def TestOne():
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
    print("Points: ", p, " X: ", p.lat)
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
    # This is an unusual function. 
    d = p.destination(Range, Bearing)
    print("New destitation: ", d)

def TestTwo():
    # different method, pGeo, don't remember how I got here. 
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
    az,R = pGeo.RangeBearing(Lon, Lat, LonRI, LatRI)
    print("Azimuth: ", az," Range(m):", R)
    
def TestThree():
    # Google Maps position for 217 Locust Ave
    Lat =  41.308489
    Lon = -73.893027
    CalculateLimitsTwo(Lat,Lon, 1000)
    
if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """

    #TestOne()
    #TestTwo()
    TestThree()
