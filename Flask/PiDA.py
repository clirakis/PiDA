"""
 ===============================================================
Modified    By    Reason
--------    --    ------
08-Dec-23   CBL   Commenting
13-Dec-23   CBL   Basic layout done, adding in some buttons. 
14-Dec-23   CBL   Moved over to PiDA and implementing.
17-Dec-23   CBL   Added in the potential for plotting, also made this a python
                  start

Bootstrap has a lot of stuff in it, basic templates
moment is for dealing with time 

References:
https://pypi.org/project/Flask-Bootstrap/
https://flask-moment.readthedocs.io/en/latest/quickstart.html#rendering-timestamps-with-flask-moment

 ===============================================================
"""
from flask import (
    Flask, render_template, send_file, make_response, request, redirect,
    url_for
    )
from flask_bootstrap import Bootstrap
from flask_moment import Moment
from datetime import datetime
import os
import numpy as np
from Plot.Graph import Graph

# My local imports
from PySM.NMEA_GGA import NMEA_GGA
from PySM.NMEA_RMC import NMEA_RMC
from PySM.NMEA_GSA import NMEA_GSA
from PySM.NMEA_VTG import NMEA_VTG
from PySM.IMU      import IMU


app = Flask(__name__)

"""
Everything is global, probably ought to tighten this up.
"""

# This is straight from the documentation and should work. 
bootstrap = Bootstrap(app)
moment    = Moment(app)
# create the classes
MyGGA = NMEA_GGA()
#MySM = NMEA_GSA()
#MySM = NMEA_RMC()
MyVTG = NMEA_VTG()
MyIMU  = IMU()

"""
Initialize the ploting package.
"""
MyGraph = Graph()


def FixStr(fixType):
    """
    @param fixType - numerical value of fix
    @return fixstr - string representing type of fix. 
    """
    if (fixType == 0):
        fixstr = 'INVALID'
    elif (fixType == 1):
        fixstr = 'GPS'
    elif (fixType == 2):
        fixstr = 'DGPS'
    elif (fixType == 3):
        fixstr = 'N/A'
    elif (fixType == 4):
        fixstr = 'RTK Fixed'
    elif (fixType == 5):
        fixstr = 'RTK FLOAT'
    elif (fixType == 6):
        fixstr = 'INS DR'
    else:
        fixstr = 'UNKOWN'
    return fixstr

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

@app.route('/')
def index():
    return render_template('index.html',current_time=datetime.utcnow()) 

@app.route('/user')
def user():
    return render_template('user.html',current_time=datetime.utcnow())

@app.route('/gps',methods=['GET','POST'])
def gps():

    if request.method == 'POST':
        # POST is the form sent some data. 
        print('GPS POST')

        if request.form.get('refresh') == 'Refresh':
            print("GPS do refresh")
        else:
            print("method unknown")

    elif request.method == 'GET':
        # GET is a give me some data
        print('GPS GET')

    MyGGA.Read()
    MyVTG.Read()

    #
    # speed is in Knots.
    #
    speed = MyVTG.fSpeedKnots * 1852.0/3600.0
    #
    # format the data into strings. Too many digits otherwise.
    #
    lat    = np.rad2deg(MyGGA.fLatitude)
    lon    = np.rad2deg(MyGGA.fLongitude)
    slat   = '{:06.6f}'.format(lat)
    slon   = '{:06.6f}'.format(lon)
    salt   = '{:06.2f}'.format(MyGGA.fAltitude)
    sgeo   = '{:04.2f}'.format(MyGGA.fGeoidSep)
    sspeed = '{:03.2f}'.format(speed)
    sTrue  = '{:03.1f}'.format(MyVTG.fTrue)
    sMag   = '{:03.1f}'.format(MyVTG.fMagnetic)

    if (lat>0.0):
        MyGraph.AddPoint( lon, lat)

    return render_template('GPS.html',current_time=datetime.utcnow(),
                           Latitude=slat,
                           Longitude=slon,
                           Altitude=salt,
                           Geiod=sgeo,
                           FixTime=MyGGA.fSec,
                           Speed=sspeed,
                           CompassTrue=sTrue,
                           CompassMagnetic=sMag,
                           FOffset=0.0,
                           FixType=FixStr(MyGGA.fFix)
                           )

@app.route('/imu', methods=['GET','POST'])
def imu():
    if request.method == 'POST':
        # POST is the form sent some data. 
        print('IMU POST')

        if request.form.get('refresh') == 'Refresh':
            print("IMU do refresh")
        else:
            print("method unknown")

    elif request.method == 'GET':
        # GET is a give me some data
        print('IMU GET')
    MyIMU.Read()

    sTemperature = '{:03.1f}'.format(MyIMU.fTemperature)
    sAx = '{:06.6f}'.format(MyIMU.fAcc[0])
    sAy = '{:06.6f}'.format(MyIMU.fAcc[1])
    sAz = '{:06.6f}'.format(MyIMU.fAcc[2])
    sGx = '{:06.6f}'.format(MyIMU.fGyro[0])
    sGy = '{:06.6f}'.format(MyIMU.fGyro[1])
    sGz = '{:06.6f}'.format(MyIMU.fGyro[2])
    sMx = '{:06.6f}'.format(MyIMU.fMagnetic[0])
    sMy = '{:06.6f}'.format(MyIMU.fMagnetic[1])
    sMz = '{:06.6f}'.format(MyIMU.fMagnetic[2])

    
    return render_template('IMU.html',
                           current_time=datetime.utcnow(),
                           IMUTime=datetime.utcnow(),
                           Fraction=0.1,
                           Temperature=sTemperature,
                           AX=sAx,AY=sAy,AZ=sAz,
                           GX=sGx,GY=sGy,GZ=sGz,
                           MX=sMx,MY=sMy,MZ=sMz,
                           )

@app.route('/testme')
def testme():
    return render_template('testme.html')

@app.route('/plotGPS')
def plotGPS():
    global MyGraph
    graphdata = MyGraph.InlinePlot()
    response = make_response(graphdata.getvalue())
    response.mimetype = 'image/png'
    return response

if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    # put my initialization in here. This one  
    #
    # Plotting tools.
    #
    moment.init_app(app)
    app.run(host='0.0.0.0', port=5000, debug=True)
