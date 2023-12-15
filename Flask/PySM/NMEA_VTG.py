"""@NMEA_VTG
  Interface using the POSIX interface of shared memory for attaching
  to the NMEA VTG class. This inherits from the base class of
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
import numpy as np

class NMEA_VTG(SharedMem2):
    def __init__(self):
        # 28 total bytes used for base class
        #
        params = {'name':'VTG', 'size': 20, 'server': False}
        # self is implied when using super.
        super().__init__(params)
        
        # VTG Specific
        self.fTrue       = 0.0   # 
        self.fMagnetic   = 0.0   # 
        self.fSpeedKnots = 0.0   # 
        self.fSpeedKPH   = 0.0   # 
        self.fMode       = 0.0   # fix mode

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure.
        The VTG message is a subclass to NMEA_POSITION.
        See GTOP/NMEA_GPS.hh
        """
        super().Read()
        
        # VTG specific
        self.fTrue       = self.Unpack('f')
        self.fMagnetic   = self.Unpack('f')
        self.fSpeedKnots = self.Unpack('f')
        self.fSpeedKPH   = self.Unpack('f')
        self.fMode       = self.Unpack('b')
        
        self.UnpackDone()
        if (self.debug):
            print("SELF: ",self)


    def Print(self):
        print('NMEA VTG')
        print('    Compass True: ', self.fTrue)
        print('Compass Magnetic: ', self.fMagnetic)
        print('   Speed (Knots): ', self.fSpeedKnots)
        print("     Speed (KPH): ", self.fSpeedKPH)
        print("           fMode: ", self.fMode)

    def __str__(self):
        rep  = "NMEA_VTG --------------------------------------------" + "\n"
        rep += "     Compass True: " + str(self.fTrue) + "\n"
        rep += " Compass Magnetic: " + str(self.fMagnetic) + "\n"
        rep += "     Speed(Knots): " + str(self.fSpeedKnots) + "\n"
        rep += "       Speed(KPH): " + str(self.fSpeedKPH) + "\n"
        rep += "             Mode: " + str(self.fMode) + "\n"
        rep += " ------------------------------------------------------"
        return rep
        
