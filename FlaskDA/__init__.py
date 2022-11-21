"""
 ===============================================================
 
 All initialzation is done here.

  Modified  By  Reason
  --------  --  ------
 13-Nov-22  CBL Added in some additional information for standard init.
 20-Nov-22  CBL Added all of the main module here out of FlaskDA.py
 
 ===============================================================
"""
import os
import sys
from flask import (
    Flask, render_template, send_file, make_response, request, redirect,
    url_for
    )
import numpy as np
from flask_bootstrap import Bootstrap
from flask_moment    import Moment
from datetime        import datetime

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

def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    app.config.from_mapping(
        SECRET_KEY='dev',
    )

    bootstrap = Bootstrap(app)
    moment    = Moment(app)
    moment.init_app(app)

    # Setup the search path for the modules.
    #sys.path.insert(0, app.instance_path)

    """
    ===============================================================
    put my initialization in here. This one  
    Initialize the ploting package.
    """
    PPlot = ProjectedPlot(41.3,-73.83)
    PPlot.Center(41.308385, -73.893)

    """
    Attaches to the time position shared memory if it exists.
    This is based on POSIX shared memory and is tightly linked
    to a binary data acquisition module I have called lassen.
    """
    MySM = TSIPosition()
    
    
    # ===============================================================

    if test_config is None:
        # load the instance config, if it exists, when not testing
        app.config.from_pyfile('config.py', silent=True)
    else:
        # load the test config if passed in
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass
    
    def dataThread(pp):
        """
        Loop and check for new GPS data for the plots. 
        """
        t0 = time.time()
        print("THREAD STARTS: ", t0)
        running = True
        count = 0
    
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
        
            print("get fix: ", t1, " ", count)
            count = count + 1
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
        output = PPlot.InlinePlot()
        response = make_response(output.getvalue())
        response.mimetype = 'image/png'
        return response

    # ===============================================================
    """
    Setup signals.
    """
    signal.signal(signal.SIGALRM, SignalHandler)
    signal.signal(signal.SIGINT, SignalHandler)
    #signal.alarm(1)

    """
    Start the thread.
    """
    # for use later in the thread
    running = False
    x = threading.Thread(target=dataThread, args=(1,))
    x.start()

    
    return app
