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

  17-Nov-22 CBL I don't think using an alarm funciton is the correct way to
                add points to the graph, changing to thread. 
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
import threading 
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
#
# Global variables.
#
global running
global MySM
global PPlot

#  ===============================================================


def dataThread(pp):
    """
    Loop and check for new GPS data for the plots. 
    """
    global running
    global MySM
    global PPlot

    t0 = time.time()
    running = True

    
    """
    attaches to the
    time position shared memory if it exists.
    This is based on POSIX shared memory and is tightly linked
    to a binary data acquisition module I have called lassen.
    """
    MySM = TSIPosition()

    while running:
        """
        Look at the loop time, make sure we aren't doing anything
        radically wrong.
        """
        t1 = time.time()
        dt = t1-t0
        t0 = t1
        """
        Get the fix if there is new data.
        """
        if (MySM.NoError()):
            MySM.Read()
        else:
            print('Error in reading from SM.')
        
        print("get fix: ", dt)
        # Timeout varies a shade depending on how long the processing takes
        x = np.rad2deg(MySM.fLongitude)
        y = np.rad2deg(MySM.fLatitude)
        PPlot.addPoint(x,y)
        time.sleep(0.9)
        
def SignalHandler(signum, frame):
    """
    Handle any signals here
    """
    global running
    if (signum == signal.SIGALRM) :
        print ('Alarm')
    elif (signum == signal.SIGINT):
        print("CTRL-C")
        running = False
        signal.alarm(0)
        exit(0)

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
    global PPlot
    global MySM
    
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

    LatDeg,LatMin,LatSec = MySM.Format(MySM.fLatitude)
    LonDeg,LonMin,LonSec = MySM.Format(MySM.fLongitude)

    #gps_t, fix_time, Lat, Lon, z = getGPS_Position()
    templateData = {
        'time': time.ctime(MySM.fSec),
        'LatDeg' : LatDeg,
        'LatMin' : LatMin,
        'LatSec' : LatSec,
        'LonDeg' : LonDeg,
        'LonMin' : LonMin,
        'LonSec' : LonSec,
        'Alt' : round(MySM.fAltitude,2),
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
    global PPlot
    output = PPlot.InlinePlot()
    response = make_response(output.getvalue())
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
    """
    Initialize the ploting package.
    """
    PPlot = ProjectedPlot(41.3,-73.83)
    PPlot.Center(41.308385, -73.893)
    """
    Setup signals.
    """
    signal.signal(signal.SIGALRM, SignalHandler)
    signal.signal(signal.SIGINT, SignalHandler)
    #signal.alarm(1)
    x = threading.Thread(target=dataThread, args=(1,))
    x.start()
    moment.init_app(app)
    app.run(host='0.0.0.0', port=8050, debug=True,threaded=True)
