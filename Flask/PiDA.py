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

app = Flask(__name__)

# This is straight from the documentation and should work. 
bootstrap = Bootstrap(app)
moment    = Moment(app)

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
    return render_template('GPS.html',current_time=datetime.utcnow(),
                           Latitude=41.5,Longitude=71.3)

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
        
    return render_template('IMU.html',current_time=datetime.utcnow(),
                           IMUTime=datetime.utcnow(),
                           Fraction=0.1,
                           Temperature=26.5,MX=0.1,MY=0.6,MZ=0.01)

@app.route('/testme')
def testme():
    return render_template('testme.html')
