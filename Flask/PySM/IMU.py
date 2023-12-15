"""@IMU
  Interface using the POSIX interface of shared memory for attaching
  to the IMU class. This inherits from the base class of
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

class IMU(SharedMem2):
    def __init__(self):
        # 100 total bytes used for base class
        #
        params = {'name':'IMU', 'size': 100, 'server': False}
        # self is implied when using super.
        super().__init__(params)
        
        # VTG Specific
        self.fAcc        = np.Zeros(3)
        self.fMagnetic   = np.zeros(3)
        self.fGyro       = np.zeros(3)
        self.fTemperature= 0.0
        self.fSuccess    = False

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure.
        The IMU is non-specific
        See ICM-20948 
        """
        super().Read()
        
        # VTG specific
        self.fAcc[0]     = self.Unpack('d')
        self.fAcc[1]     = self.Unpack('d')
        self.fAcc[2]     = self.Unpack('d')
        self.fMagnetic[0]= self.Unpack('d')
        self.fMagnetic[1]= self.Unpack('d')
        self.fMagnetic[2]= self.Unpack('d')
        self.fGyro[0]    = self.Unpack('d')
        self.fGyro[1]    = self.Unpack('d')
        self.fGyro[2]    = self.Unpack('d')
        self.fTemperature= self.Unpack('d')
        self.fSuccess    = self.Unpack('B')
        
        self.UnpackDone()
        if (self.debug):
s            print("SELF: ",self)

    def Print(self):
        print('IMU')
        print(' Acceleration(g) X: ', self.fAcc[0], " Y:", self.fAcc[1],
              " Z:", self.fAcc[2] )
        print(' Magnetic X: ', self.fMagnetic[0], " Y:", self.fMagnetic[1],
              " Z:", self.fMagnetic[2] )
        print(' Gyro (rad/sec) X: ', self.fGyro[0], " Y:", self.fGyro[1],
              " Z:", self.fGyro[2] )
        print(' Temperature: ', self.fTemperature)


    def __str__(self):
        rep  = "IMU --------------------------------------------" + "\n"
        rep += "     Acceleration: " + str(self.fAcc[0]) + str(self.fAcc[1]) + str(self.fAcc[2])+ "\n"
        rep += "         Magnetic: " + str(self.fMagnetic[0]) + str(self.fMagnetic[1]) + str(self.fMagnetic[2])+ "\n"
        rep += "             Gyro: " + str(self.fGyro[0]) + str(self.fGyro[1]) + str(self.fGyro[2])+ "\n"
        rep += "      Temperature: " + str(self.fTemperature) << "\n"
        rep += " ------------------------------------------------" << "\n"
        return rep
        
