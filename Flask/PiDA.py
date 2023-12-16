"""
 ===============================================================
Modified    By    Reason
--------    --    ------
08-Dec-23   CBL   Commenting
13-Dec-23   CBL   Basic layout done, adding in some buttons. 
14-Dec-23   CBL   Moved over to PiDA and implementing.

Bootstrap has a lot of stuff in it, basic templates
moment is for dealing with time 

References:
https://pypi.org/project/Flask-Bootstrap/
https://flask-moment.readthedocs.io/en/latest/quickstart.html#rendering-timestamps-with-flask-moment

 ===============================================================
"""
from flask import Flask, render_template, request
from flask_bootstrap import Bootstrap
from flask_moment import Moment
from datetime import datetime
import os
import numpy as np

# My local imports
from PySM.NMEA_GGA import NMEA_GGA
from PySM.NMEA_RMC import NMEA_RMC
from PySM.NMEA_GSA import NMEA_GSA
from PySM.NMEA_VTG import NMEA_VTG
from PySM.IMU      import IMU


app = Flask(__name__)

# This is straight from the documentation and should work. 
bootstrap = Bootstrap(app)
moment    = Moment(app)
# create the classes
MyGGA = NMEA_GGA()
#MySM = NMEA_GSA()
#MySM = NMEA_RMC()
MyVTG = NMEA_VTG()
MyIMU  = IMU()

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
    
    return render_template('GPS.html',current_time=datetime.utcnow(),
                           Latitude=np.rad2deg(MyGGA.fLatitude),
                           Longitude=np.rad2deg(MyGGA.fLongitude),
                           Altitude=MyGGA.fAltitude,
                           Geiod=MyGGA.fGeoidSep,
                           FixTime=MyGGA.fSec,
                           Speed=speed,
                           CompassTrue=MyVTG.fTrue,
                           CompassMagnetic=MyVTG.fMagnetic,
                           FOffset=0.0,
                           FixType=MyGGA.fFix
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
        
    return render_template('IMU.html',
                           current_time=datetime.utcnow(),
                           IMUTime=datetime.utcnow(),
                           Fraction=0.1,
                           Temperature=MyIMU.fTemperature,
                           AX=MyIMU.fAcc[0],AY=MyIMU.fAcc[1],AZ=MyIMU.fAcc[2],
                           GX=MyIMU.fGyro[0],GY=MyIMU.fGyro[1],
                           GZ=MyIMU.fGyro[2],
                           MX=MyIMU.fMagnetic[0],MY=MyIMU.fMagnetic[1],
                           MZ=MyIMU.fMagnetic[2],
                           )

@app.route('/testme')
def testme():
    return render_template('testme.html')
