"""@Commands
  Interface using the POSIX interface of shared memory for attaching
  to the commands. This inherits from the base class of
  SharedMem2.py.

  Allow the parent process to consume the current filename and
  send commands to the GPS. The starting one is to change filenames.

     Modified  By   Reason
     --------  --   ------
     22-Feb-26 CBL  Original


  References:
  https://www.programiz.com/python-programming/inheritance

  Unit Tested: 

 ====================================================================
"""
from PySM.SharedMem2 import SharedMem2
import math
import numpy as np

class Commands(SharedMem2):
    def __init__(self):
        # 512 total bytes used for base class
        # At the moment this is setup to send messages
        # not necessarily recieve replies. I should 
        #
        params = {'name':'GPS_Commands', 'size': 512, 'server': False}
        # self is implied when using super.
        super().__init__(params)
        self.filename    = 'NONE'
        self.fSuccess    = False

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        not sure how to handle this yet to read and replies from commands
        """
        super().Read()
        
        self.UnpackDone()
        if (self.debug):
            print("SELF: ",self)

    def Print(self):
        print('Commands')
        print(' Filename: ', self.filename)

    def __str__(self):
        rep  = "Commands ----------------------------------------" + "\n"
        rep += "     Filename:" + self.filename + "\n"
        rep += " ------------------------------------------------" + "\n"
        return rep
        
# CF - Change Filename
# GF - get filename
