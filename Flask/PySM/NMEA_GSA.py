"""@NMEA_GSA
  Interface using the POSIX interface of shared memory for attaching
  to the NMEA GSA class. This inherits from the base class of
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

class NMEA_GSA(SharedMem2):
    def __init__(self):
        # 28 total bytes used for base class
        #
        params = {'name':'GSA', 'size': 28, 'server': False}
        # self is implied when using super.
        super().__init__(params)
        
        # GSA Specific
        self.fPDOP      = 0.0   # 
        self.fHDOP      = 0.0   # knots 
        self.fVDOP      = 0.0   # course made good  
        self.fMode1     = 0.0   # fix mode
        self.fMode2     = 0.0   # fix mode
        self.fSatellites = np.zeros(12)

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure.
        The GSA message is a subclass to NMEA_POSITION.
        See GTOP/NMEA_GPS.hh
        """
        super().Read()
        
        # GSA specific
        self.fMode1     = self.Unpack('b')
        self.fMode2     = self.Unpack('b')
        space1          = self.Unpack('b')
        space2          = self.Unpack('b')
        for index in range(10):
            fSatellites[index] = self.Unpack('b')
        self.fPDOP      = self.Unpack('f')
        self.fHDOP      = self.Unpack('f')
        self.fVDOP      = self.Unpack('f')
        
        self.UnpackDone()
        if (self.debug):
            print("SELF: ",self)


    def Print(self):
        print('NMEA GSA')
        print('Mode 1: ', self.fMode1)
        print('Mode 2: ', self.fMode2)
        print('Satellites:', end=' ')
        for index in range(10):
            print(self.fSatellites[index],end=' ')
        print(" ")
        print("PDOP: ", self.fPDOP)
        print("HDOP: ", self.fHDOP)
        print("VDOP: ", self.fVDOP)

    def __str__(self):
        rep  = "NMEA_GSA --------------------------------------------" + "\n"
        rep += " Mode 1: " + str(self.fMode1) + "\n"
        rep += " Mode 2: " + str(self.fMode2) + "\n"
        rep += " Satellites: "
        for index in range(10):
            rep += str(self.fSatellites[index]) + " "
        rep += "\n"
        rep += "PDOP: " + str(self.fPDOP) + "\n"
        rep += "HDOP: " + str(self.fHDOP) + "\n"
        rep += "VDOP: " + str(self.fVDOP) + "\n"
        
        rep += " ------------------------------------------------------"
        return rep
        
