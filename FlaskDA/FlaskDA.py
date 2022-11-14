"""
 FlaskDA.py

  Modified  By  Reason
  --------  --  ------
  03-Feb-22 CBL Original, originates from web links below start of bringing
                all the pieces together for a DA system
                
  05-Feb-22 CBL Operational, adding in plotting. 

  09-Oct-22 CBL adding functionality. Still doing testing with the lassen GPS.

  06-Nov-22 CBL Menu packages aren't really that great. Adding more buttons
                and neating up the table of data.
                Need to set the limits by scale value.
                A lot of what is below replicates. Can I fix it to
                call a single template call. Also really need to work on
                the sizing of the label/numberical entry and set button.
                
  13-Nov-22 CBL condensed the way of handling callbacks/button pushes etc. 
                Added in bootstrap for menus and template inheritance
  References:
  -----------
   https://medium.com/@rovai/from-data-to-graph-a-web-jorney-with-flask-and-sqlite-6c2ec9c0ad0
  https://wiki.gentoo.org/wiki/Gpib

 https://flask.palletsprojects.com/en/2.2.x/api/
 https://flask.palletsprojects.com/en/2.2.x/api/#url-route-registrations

 ===============================================================
 """
import os
from flask import (
    Flask, render_template, send_file, make_response, request, redirect,
    url_for
    )
import numpy as np
from flask_bootstrap import Bootstrap
from flask_moment import Moment
from datetime import datetime

#from jinja2.ext import Extension
#from jinja2 import Template
from threading import Thread
import time
import signal
#  ===============================================================
# My methods
#
# access shared memory for lassen GPS (on linux box Rod)
from PySM.TSIPosition import TSIPosition
# My plotting tools. 
from Plotting.ProjectedPlot import ProjectedPlot

#  ===============================================================
# instantiate the Flask app
app = Flask(__name__)

#  ===============================================================
# bootstrap allows us to have some standard templates to operate with. 
bootstrap = Bootstrap(app)

#  ===============================================================
# date formatting etc. 
moment = Moment(app)


#  ===============================================================
# put my initialization in here. This one attaches to the
# time position shared memory if it exists.
# This is based on POSIX shared memory and is tightly linked
# to a binary data acquisition module I have called lassen. 
MySM = TSIPosition()
#
# Plotting tools.
#
PPlot = ProjectedPlot(41.3,-73.83)

#
# Global variables.
#
SleepTime = 1 # seconds between signals.
global t0
t0 = time.time()

#  ===============================================================

def getGPS_Position():
    """
    This interfaces to the shared memory of the lassen GPS using POSIX
    shared memory and semaphores. This is all managed by PySharedMem2.py
    return the
    time of fix in gps seconds
    time of fix read in seconds since epoch
    latitude in radians
    longitude in radians
    altitude in meters from the mean geoid
    """
    # Initialize variables
    gps_time = 0
    lat = 0.0
    lon = 0.0
    z = 0.0
    
    if (MySM.NoError()):
        MySM.Read()
        gps_time = MySM.fSec  # This isn't really the time
        fix_time = MySM.LastRead_Time_tv_sec
        lat  = MySM.fLatitude
        lon  = MySM.fLongitude
        z    = MySM.fAltitude
    else:
        print('Error in reading from SM.')
        gps_time = 0
        fix_time = time.time()
        lat = np.deg2rad(41.5)
        lon = np.deg2rad(-71.2)
        z = 0.0
    return gps_time, fix_time, lat, lon, z

def SignalHandler(signum, frame):
    """
    Wake up and check for new GPS data for the plots. 
    """
    global t0
    if (signum == signal.SIGALRM) :
        t1 = time.time()
        dt = t1-t0
        t0 = t1
        gps_t, fix_time, Lat, Lon, z = getGPS_Position()
        #print("Timeout: ", dt)
        x = np.rad2deg(Lon)
        y = np.rad2deg(Lat)
        PPlot.addPoint(x,y)
        # make it happen again. 
        signal.alarm(1)

@app.errorhandler(404)
def page_not_found(e):
    """
    our own handler for 404. The resulting page is found in
    templates/404.html
    """
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

@app.route("/about")
def about():
    """
    Tell the user about the application.
    """
    print("ABOUT")
    return render_template('about.html')
    

# main route - home page
@app.route("/", methods=['POST', 'GET'])
def index():
    """
    Parent form data. handle index.html at top.
    Make sure the form action="/" and it will be processed here.
    First determine if it is a post or get, then switch on response based
    on the determination. 
    """
    if request.method == 'POST':
        val = request.form.get('Submit')
        print("POST!", val)
        if(val == 'Set Scale'):
           data = request.form.get("fScale")
           x = float(data)
           print('Set Scale.', x)
           PPlot.Scale(x)
        elif(val == 'Set Grid'):
            data = request.form.get("gridtype")
            PPlot.setGrid(data)
            print("Set Grid:", data)
        
    elif request.method == 'GET':
        print("GET!")
    
    gps_t, fix_time, Lat, Lon, z = getGPS_Position()
    LatDeg,LatMin,LatSec = MySM.Format(Lat)
    LonDeg,LonMin,LonSec = MySM.Format(Lon)
    templateData = {
        'time': time.ctime(fix_time),
        'LatDeg' : LatDeg,
        'LatMin' : LatMin,
        'LatSec' : LatSec,
        'LonDeg' : LonDeg,
        'LonMin' : LonMin,
        'LonSec' : LonSec,
        'Alt' : round(z,2),
        'gridtype' : PPlot.whichGrid(),
        'fscale' : PPlot.whichScale(),
    }
    return render_template('index.html', **templateData)



@app.route('/plot/scat')
def plot_scat():
    """@plot_scat
    Gets an inline png to plot from Position Plot and returns it
    to be rendered. 
    """
    output = PPlot.InlinePlot()
    response = make_response(output.getvalue())
    response.mimetype = 'image/png'
    return response



if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    PPlot.Center(41.308385, -73.893)
    signal.signal(signal.SIGALRM, SignalHandler)
    signal.alarm(1)
    moment.init_app(app)
    app.run(host='0.0.0.0', port=8050, debug=True)
