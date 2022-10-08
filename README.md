# PiDA
Data acquisition using Raspberry Pi

08-Oct-22 Putting this together for the first time. This is dependent on my common files. 
The configuration is a Raspberrey Pi4  running raspian and has a GTOP GPS receiver 
In addition there is a 9 DOF sensor Magnetic, Acceleration, RPH 
Initial directories are:
GTOP - interface and logging for GTOP GPS receiver
ICM-20948  -- ICM20948 9DOF sensor
FlaskDA -- interface to plot etc to the screen
    -- Plotting
	- CompassWidget
	- PositionPlot.py
    -- PySM 
	- SharedMem2.py - base class interface to posix shared memory
	- GPSFilename.py - get the GPS Filename
	- TSIPConstants.py
	- TSIPPosition.py
	- TSIPStatus.py
	- TSIPVelocity.py
    -- static
	style sheet
    -- templates
	index.html

GTOP uses NMEA library which is made in the GTOP directory and provided by 
Adafruit

Processor -- combine all the resources. note this uses wiring2pi