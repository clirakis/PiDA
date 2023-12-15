"""@NMEA_RMC
  Interface using the POSIX interface of shared memory for attaching
  to the NMEA RMC class. This inherits from the base class of
  SharedMem2.py.

     Modified  By   Reason
     --------  --   ------
     15-Dec-23 CBL  Original


  References:
  https://www.programiz.com/python-programming/inheritance

  Unit Tested: 

 ====================================================================
"""
from PySM.SharedMem2 import SharedMem2
import math

class NMEA_RMC(SharedMem2):
    def __init__(self):
        # RMC class inherits from NMEA_POSITION - 
        # 64 total bytes used for base class
        #
        params = {'name':'RMC', 'size': 64, 'server': False}
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

        # add ons from RMC
        self.fDelta     = 0.0   # platform and GPS time difference
        self.fSpeed     = 0.0   # knots 
        self.fCMG       = 0.0   # course made good  
        self.fMagVar    = 0.0   # magnetic variation
        self.fMode      = 0.0   # fix mode

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure.
        The RMC message is a subclass to NMEA_POSITION.
        See GTOP/NMEA_GPS.hh
        """
        super().Read()
        pCTime_Sec      = self.Unpack('l')  # ok - don't care about these
        pCTime_nSec     = self.Unpack('l')  # ok
        
        self.fLatitude  = self.Unpack('f')  # ok
        self.fLongitude = self.Unpack('f')  # ok
        self.fSec       = self.Unpack('l')  # ok, this is indeed 0 for RMC. 
        self.fUTC       = self.Unpack('l')  # ok
        self.fMilli     = self.Unpack('f')  # ok
        
        # RMC add on. 
        self.fDelta     = self.Unpack('f') # ok
        self.fSpeed     = self.Unpack('f') # ok
        self.fCMG       = self.Unpack('f') # fix indicator, ok
        self.fMagVar    = self.Unpack('f') # ok
        self.fMode      = self.Unpack('b')
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
        print('NMEA RMC')
        deg,min,sec = self.Format(self.fLatitude)
        print(' Latitude:', deg, " ", min, " ", sec)        
        deg,min,sec = self.Format(self.fLongitude)
        print('Longigude:', deg, " ", min, " ", sec) 
        print('Altitude:', round(self.fAltitude,3))
        print('Seconds:',  self.fSec)
        print('UTC (day):',self.fUTC)
        print('mSeconds:', self.fMilli)
        
        print('GPS/Platform time delta:', self.fDelta)
        print('Speed:',    self.fSpeed)
        print('CMG:',      self.fCMG)
        print('Mag Var:',  self.fMagVar)
        print('Mode:',     self.fMode)

    def __str__(self):
        rep  = "NMEA_RMC --------------------------------------------" + "\n"
        rep += " Latitude: "    + str(math.degrees(self.fLatitude)) 
        rep += " Longigude: "   + str(math.degrees(self.fLongitude)) + "\n"
        rep += " Seconds: "     + str(self.fSec) 
        rep += " milli: "       + str(self.fMilli) +"\n"
        rep += " UTC (day) "    + str(self.fUTC) +"\n"
        
        rep += " Delta: "        + str(self.fDelta) +"\n"
        rep += " Speed(knots): " + str(self.fSpeed) +"\n"
        rep += " CMG: "          + str(self.fCMG) +"\n"
        rep += " Magnetic Var: " + str(self.fMagVar) +"\n"
        rep += " Mode: "         + str(self.fMode) +"\n"
        rep += " ------------------------------------------------------"
        return rep
        
