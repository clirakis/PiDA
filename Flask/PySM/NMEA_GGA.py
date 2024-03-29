"""@NMEA_GGA
  Interface using the POSIX interface of shared memory for attaching
  to the NMEA GGA class. This inherits from the base class of
  SharedMem2.py.

     Modified  By   Reason
     --------  --   ------
     14-Dec-23 CBL  Original


  References:
  https://www.programiz.com/python-programming/inheritance

  Unit Tested: 

 ====================================================================
"""
from PySM.SharedMem2 import SharedMem2
import math

class NMEA_GGA(SharedMem2):
    def __init__(self):
        # GGA class inherits from NMEA_POSITION - 
        # 66 total bytes used for base class
        # PC time is the first 16 bytes, don't care.
        #
        params = {'name':'GGA', 'size': 66, 'server': False}
        #SharedMem2.__init__(self, params)
        # self is implied when using super.
        super().__init__(params)
        # Latitude and Longitude in radians.
        self.fLatitude  = 0.0
        self.fLongitude = 0.0 
        # Altitude, not sure of reference in meters
        self.fAltitude  = 0.0  
        # Time of Fix. 
        self.fSec       = 0
        self.fMilli     = 0.0
        self.fUTC       = 0.0
        # Add on from GGA message
        self.fFix       = 0
        self.fNSat      = 0
        self.fHDOP      = 0.0
        self.fGeoidSep  = 0.0 


    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure.
        The GGA message is a subclass to NMEA_POSITION.
        See GTOP/NMEA_GPS.hh
        """
        super().Read()
        pCTime_Sec      = self.Unpack('l')  # ok
        pCTime_nSec     = self.Unpack('l')  # ok
        
        self.fLatitude  = self.Unpack('f')  # ok
        self.fLongitude = self.Unpack('f')  # ok
        self.fSec       = self.Unpack('l')  # ok, this is indeed 0 for GGA. 
        self.fUTC       = self.Unpack('l')  # ok
        self.fMilli     = self.Unpack('f')  # ok
        
        # GGA add on. 
        self.fGeoidSep  = self.Unpack('f') # ok
        self.fAltitude  = self.Unpack('f') # ok
        self.fFix       = self.Unpack('b') # fix indicator, ok
        self.fNSat      = self.Unpack('b') # ok
        space1          = self.Unpack('b') # 
        space2          = self.Unpack('b') # 
        self.fHDOP      = self.Unpack('f') # ???
        placeholder     = self.Unpack('f')
        print("PLACEHOLDER: ",placeholder)
        self.UnpackDone()
        if (self.debug):
            print("SELF: ",self)

    def Format(self, val):
        """
        @val - value in radians to be turned in DEG MIN SEC.SSS
        3 decimal places of seconds gets us to mm values
        """
        if (val < 0):
            sign = -1.0
        else:
            sign = 1.0

        x = math.degrees(abs(val))
        deg = math.floor(x)
        x = (x-deg)*60.0
        min = math.floor(x)
        sec = round((x - min)*60.0, 3)
        return sign*deg, min, sec

    def Print(self):
        print('NMEA GGA')
        deg,min,sec = self.Format(self.fLatitude)
        print(' Latitude:', deg, " ", min, " ", sec)        
        deg,min,sec = self.Format(self.fLongitude)
        print('Longigude:', deg, " ", min, " ", sec) 
        print('Altitude:', round(self.fAltitude,3))
        print('Seconds:',  self.fSec)
        print('UTC (day):',self.fUTC)
        print('mSeconds:', self.fMilli)
        print('FIX TYPE:', self.fFix)
        print('N Sat:',    self.fNSat)
        print('HDOP:',     self.fHDOP)

    def __str__(self):
        rep  = "NMEA_GGA --------------------------------------------" + "\n"
        rep += " Latitude: "    + str(math.degrees(self.fLatitude)) 
        rep += " Longigude: "   + str(math.degrees(self.fLongitude)) + "\n"
        rep += " Altitude: "    + str(self.fAltitude)
        rep += " Geoid Sep: "   + str(self.fGeoidSep) + "\n"
        rep += " Seconds: "     + str(self.fSec) 
        rep += " milli: "       + str(self.fMilli) +"\n"
        rep += " UTC (day) "    + str(self.fUTC) +"\n"
        rep += " Fix type: "    + str(self.fFix) +"\n"
        rep += " N Satellite: " + str(self.fNSat) +"\n"
        rep += " HDOP: "        + str(self.fHDOP) +"\n"
        rep += " ------------------------------------------------------"
        return rep
        
