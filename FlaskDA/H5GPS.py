"""@H5GPS.py
   Class to read the HDF5 GPS log files. 

   Modified  By   Reason
   --------  --   ------
   08-Feb-22 CBL  Original
   
   References:
   -----------
   https://docs.h5py.org/en/stable/
   https://docs.h5py.org/en/stable/quick.html
   https://pythonnumericalmethods.berkeley.edu/notebooks/chapter11.05-HDF5-Files.html
   
"""
import h5py  as h5py
import numpy as np

class H5GPS(object):
    def __init__(self,filename):
        """
        Initialize the system.
        """
        self.Filename = None
        self.fd = []
        self.open(filename)

    def __str__(self):
        """
        Print out the salient features.
        """
        rep  = "Filename:" + str(self.Header[0]) + "\n"
        rep += " Created: " + str(self.Header[1])+ "\n"
        rep += "   Title: " + str(self.Header[2])+ "\n"
        rep += "Variable Names: " + str(self.Variables[0])+ "\n"
        rep += "N Entries: " + str(self.NEntries)+ "\n"
        rep += "Logger Version:" + str(self.Version[0])+ "\n"
        return rep
    
    def __del__(self):
        """
        Destructor - close the file.
        """
        # think this is already closed by the time it reaches here. 
        #self.close()
    
    def open(self, Filename):
        """
        Open the file provided.
        """

        try:
            self.fd        = h5py.File(Filename, 'r')
            self.Header    = self.fd['H5Logger_Header']
            self.Variables = self.fd['H5Variable_Descriptions']
            FinalState     = self.fd['H5_FinalStateInformation']
            self.NEntries  = float(FinalState[0])
            self.Version   = self.fd['H5_VersionInformation']
            self.Filename  = Filename
            return True
        except:
            self.fd        = None
            return False
        
    def read(self):
        """
        Kinda read the actual user data.
        """
        self.data = self.fd['H5_UserData']

    def Data(self, variable):
        """
        variable - a numerical index into the variable set in the file.
            Variable descriptions
            'Time:Lat:Lon:Z:NSV:PDOP:HDOP:VDOP:TDOP:VE:VN:VZ'
        """
        return np.float32(self.data[variable])
        
    def close(self):
        """
        Close the data file.
        """
        self.fd.close()
        return True

