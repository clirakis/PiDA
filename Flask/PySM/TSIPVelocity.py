"""@TSIPVelocity
  Interface using the POSIX interface of shared memory for attaching
  to the TSIP velocity class. This inherits from the base class of
  SharedMem2.py.

     Modified  By   Reason
     --------  --   ------
     11-Oct-20 CBL  Original


  References:
  https://www.programiz.com/python-programming/inheritance

  Unit Tested: 11-Oct-20

 ====================================================================
"""
from PySM.SharedMem2 import SharedMem2
import math

class TSIPVelocity(SharedMem2):
    def __init__(self):
        # 40 bytes used for base class 
        params = {'name':'GPS_Velocity', 'size': 20, 'server': False}

        # self is implied when using super.
        super().__init__(params)

        # Velocity East in meters per second (float)
        self.fEast = 0.0
        # Velocity North in meters per second (float)
        self.fNorth = 0.0
        # Velocity up in meters per second (float)
        self.fUp = 0.0
        # Clock bias rate in seconds per second (float)
        self.fClkBiasRate = 0.0
        # Time in seconds when this information was established (float)
        self.fSec = 0.0

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure. 
        """
        super().Read()

        self.fEast  = self.Unpack('f')
        self.fNorth = self.Unpack('f')
        self.fUp    = self.Unpack('f')
        self.fClkBiasRate = self.Unpack('f')
        self.fSec   = self.Unpack('f')
        self.UnpackDone()

    def Print(self):
        print('TSIP Velocity (m/s) East:', self.fEast)
        print('Velocity North (m/s):', self.fNorth)
        print('Velocity UP (m/s):',self.fUp)
        print('Clock Bias Rate:', self.fClkBiasRate)
        print('Seconds:',self.fSec)

    def __str__(self):
        rep  = "TSIP Velocity (m/s) East: " + str(self.fEast)
        rep += " North: " + str(self.fNorth)
        rep += " Up: " + str(self.fUp) + "\n"
        rep += "Clock Bias Rate: " + str(self.fClkBiasRate)
        rep += " Seconds: " + str(self.fSec)
        return rep
        
