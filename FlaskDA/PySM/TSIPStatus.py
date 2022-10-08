"""@TSIPStatus
  Interface using the POSIX interface of shared memory for attaching
  to the TSIP SolutionStatus class. This inherits from the base class of
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
import numpy as np
import PySM.TSIPConstants as TSIPC

class TSIPStatus(SharedMem2):
    def __init__(self):
        # 40 bytes used for base class 
        params = {'name':'GPS_Solution', 'size': 26, 'server': False}

        # self is implied when using super.
        super().__init__(params)

        # Precision Dilution of Precision (float)
        self.fPDOP = 0.0
        # Horizontal Dilution of Precision (float)
        self.fHDOP = 0.0
        # Vertical Dilution of Precision (float)
        self.fVDOP = 0.0
        # Time Dilution of Precision (float)
        self.fTDOP = 0.0
        # PRN's participating in view. MAXPRNCOUNT unsigned characters
        self.fPRN = np.zeros((TSIPC.MAXPRNCOUNT,),dtype='B')  
        
        # Solution status - derived from mode, Bit 3: 0 - Auto, 1-Manual
        # Bits 0:2 3 - 2D, 4 - 3D.
        # This is byte 0 of the 0x6D report packet. (unsigned char)
        self.fSolution = 0
        
        # Number of satellites in view, derived from mode (unsigned char)
        self.fNSV = 0


    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure. 
        """
        super().Read()

        self.fPDOP  = self.Unpack('f')
        self.fHDOP  = self.Unpack('f')
        self.fVDOP  = self.Unpack('f')
        self.fTDOP  = self.Unpack('f')

        #
        # Unpack array
        #
        for i in range(TSIPC.MAXPRNCOUNT):
            self.fPRN[i] = self.Unpack('B')

        self.fSolution = self.Unpack('B')
        self.fNSV   = self.Unpack('B')
        self.UnpackDone()

    def Print(self):
        print(f'TSIP Solution Status PDOP: {self.fPDOP:.2f}')
        print(f'HDOP: {self.fHDOP:.2f}')
        print(f'VDOP: {self.fVDOP:.2f}')
        print(f'TDOP: {self.fTDOP:.2f}')
        print('NSV:',  self.fNSV)
        print('PRN:',  self.fPRN)

        ndim  = self.fSolution & int(0x03)
        nType = (self.fSolution>>3) & 1

        if (nType >0):
            typestr = 'Manual'
        else:
            typestr = 'Auto'
        if (ndim > 3):
            dimstr = '3D'
        else:
            dimstr = '2D'
        print('Dimension: ',dimstr, ' Mode: ', typestr) 

    def __str__(self):
        ndim  = self.fSolution & int(0x03)
        nType = (self.fSolution>>3) & 1
        if (nType >0):
            typestr = 'Manual'
        else:
            typestr = 'Auto'
        if (ndim > 3):
            dimstr = '3D'
        else:
            dimstr = '2D'
       
        rep  = "TSIP Status PDOP: " + str(self.fPDOP)
        rep += " HDOP: " + str(self.fHDOP)
        rep += " VDOP: " + str(self.fVDOP)
        rep += " TDOP: " + str(self.fTDOP)
        rep += " nPRN: " + str(self.fNSV)
        rep += " Solution: " + dimstr + " " + typestr
        rep += "\n"
        rep += " PRN: "  + str(self.fPRN)
        return rep
        
