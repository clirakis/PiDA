"""@TSIPosition
  Interface using the POSIX interface of shared memory for attaching
  to the TSIP position class. This inherits from the base class of
  SharedMem2.py.

     Modified  By   Reason
     --------  --   ------
     27-Sep-20 CBL  Original
     05-Feb-22 CBL  Moved into a greater python structure, added in formating


  References:
  https://www.programiz.com/python-programming/inheritance

  Unit Tested 11-Oct-20 CHECK

 ====================================================================
"""
from PySM.SharedMem2 import SharedMem2
import math

class TSIPosition(SharedMem2):
    def __init__(self):
        # 40 bytes used for base class 
        params = {'name':'GPS_Position', 'size': 36, 'server': False}
        #SharedMem2.__init__(self, params)
        # self is implied when using super.
        super().__init__(params)
        # Latitude and Longitude in radians.
        self.fLatitude  = 0.0
        self.fLongitude = 0.0 
        # Altitude, not sure of reference in meters
        self.fAltitude  = 0.0  
        # Clock bias data
        self.fClk       = 0.0
        # Time of Fix. 
        self.fSec       = 0.0
        # Fix is valid, true/false
        self.fValid     = False

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure. 
        """
        super().Read()

        self.fLatitude  = self.Unpack('d')
        self.fLongitude = self.Unpack('d')
        self.fAltitude  = self.Unpack('d')
        self.fClk       = self.Unpack('d')
        self.fSec       = self.Unpack('f')
        self.fValid     = self.Unpack('B')
        self.UnpackDone()

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
        print('TSIP Position')
        deg,min,sec = self.Format(self.fLatitude)
        print(' Latitude:', deg, " ", min, " ", sec)        
        deg,min,sec = self.Format(self.fLongitude)
        print('Longigude:', deg, " ", min, " ", sec) 
        print('Altitude:',round(self.fAltitude,3))
        print('Clock Bias:', self.fClk)
        print('Seconds:',self.fSec)
        print('Valid:',self.fValid)

    def __str__(self):
        rep  = "TSIP Position, Latitude: " + str(math.degrees(self.fLatitude))
        rep += " Longigude: " + str(math.degrees(self.fLongitude))
        rep += " Altitude: " + str(self.fAltitude) + "\n"
        rep += "Clock Bias: " + str(self.fClk)
        rep += " Seconds: " + str(self.fSec)
        rep += " Valid: " + str(self.fValid)
        return rep
        
