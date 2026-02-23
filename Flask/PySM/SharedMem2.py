"""@SharedMem2
  Interface using the POSIX interface to shared memory. 

  This is the python interface to the OH associated with the shared memory
  class I created for all my hardware interfaces.

     Modified  By   Reason
     --------  --   ------
     26-Sep-20 CBL  Original
     22-Feb-26 CBL  Need a write


  References:
  https://stackoverflow.com/questions/35988/c-like-structures-in-python
  https://pypi.org/project/posix_ipc/

 ====================================================================
"""
import mmap
import struct
import time
from typing import NamedTuple
# 3rd party modules
import posix_ipc


class SharedMem2:
##    Length : int
##    LastUpdateTime_tv_sec: int 
##    LastUpdateTime_tv_nsec: int
##    DoubleData: float
##    LAM:int 

    def __init__(self, params):
        """@brief Creates a new structure that contains the header information
        of the shared memory. The user is expected to use this as a
        base class and build on top of it. 
        @param name is the name of the shared memory segment. 
        """
        
        self.SM_name     = params.get('name')
        self.PayloadSize = params.get('size')
        self.IsServer    = params.get('server')

        self.init_complete = False
        self.error     = 0
        self.semaphore = None
        self.Mapfile   = None

        # turn this on for diagnostic printing
        self.debug     = False 

        # reset all the class variables.
        # Length is size_t
        self.Length                  = 0
        # the next two are unsigned long
        self.LastUpdate_Time_tv_sec  = 0
        self.LastUpdate_Time_tv_nsec = 0
        # this is double
        self.DoubleData              = 0.0
        # this is a bool. 
        self.LAM                     = 0

        # updated on read. 
        self.LastRead_Time_tv_sec    = 0
        self.LastRead_Time_tv_nsec   = 0

        # input buffer. Pass back to super class.
        self.inb = None
        # number of bytes processed in read. 
        self.bytes = 0
        
        if self.SM_name is None:
            print ("Provide name of shared memory to attach to.\n")
            self.error = -1
            return
        
        # number of bytes in header.
        # size_t + 2 Longs + Double + bool (but rounded up.)
        # This is very machine dependent. 40 bytes + User payload
        self.Memsize = 40 + self.PayloadSize

        SemaphoreName = 'SEM_' + self.SM_name
        if (self.debug):
            print('Semaphore Name ', SemaphoreName)
        #
        # This is the client, lets attach to both the shared memory
        # and the semaphore.
        #
        #
        try:
            self.memory    = posix_ipc.SharedMemory(self.SM_name)
            if (self.debug):
                print('Attached to memory: ', self.SM_name)
        except:
            print('Error attaching to shared memory: ', self.SM_name)
            self.error = -2
            return
        try:    
            self.semaphore = posix_ipc.Semaphore(SemaphoreName)
            if (self.debug):
                print('Attached to semaphore.', SemaphoreName)
        except:
            print("Error attaching to semaphore: ", SemaphoreName)
            self.error = -3
            return

        # Print out diagnostics
        if (self.debug):
            print('Attached to memory and semaphore.', self.memory.fd, 
                  'sm size: ', self.memory.size)
        #
        # Now memory map the data.
        #
        try:
            self.Mapfile = mmap.mmap( self.memory.fd,self.memory.size)
            if (self.debug):
                print('Attached to mmap, size: ' , self.memory.size)
        except:
            print('Error attaching to memory map.')
            self.error = -4
            return
        
        self.init_complete = True

    def __del__(self):
        """
        Destructor for SharedMem2.py
        """
        if (self.debug):
            print('SharedMem2 Cleaning up.')
        if self.semaphore is not None:
            self.semaphore.release()
            time.sleep(1)
            self.semaphore.close()
        if self.Mapfile is not None:
            self.Mapfile.close()

    def NoError(self):
        """
        NoError
        Check if the last operation raised an error.
        return true if there is no error, false if not. 
        """
        if (self.error == 0):
            return True
        else:
            return False

    def Unpack(self, format):
        """
        Unpack
        specify format
        see: https://docs.python.org/3/library/struct.html
        Regularly used formats include:
        f - float
        d - double
        l - long integer
        i - integer (signed)
        B - unsigned char
        c - character
        b - signed char
        
        Using a dictionary.
        https://stackoverflow.com/questions/60208/replacements-for-switch-statement-in-python
        
        Problem here, This is called multiple times, need to reset
        self.nbytes
        """
        nbytes = {
            'b':1,
            'B':1,
            'c':1,
            'd':8,
            'i':4,
            'I':4,
            'f':4,
            'l':8,
            }[format]

        if ((self.bytes+nbytes) > self.memory.size):
            print('SharedMem2:unpack, length exceeded',self.bytes+nbytes, " ", self.memory.size)
            return 0

        # specify start and end of byte stream to unpack.
        start = self.bytes
        end   = self.bytes + nbytes
        # advance pointer
        self.bytes += nbytes
        return struct.unpack(format, self.inb[start:end])[0]

    def UnpackDone(self):
        """
        Reset the self.bytes count.
        """
        self.bytes = 0

    def Read(self):
        """
        Read the header information.
        
        Lock the semaphore and read the data.
        """
        if (self.debug):
            print('SharedMem2 debug ================================')
            print('Acquiring semaphore.')
        self.semaphore.acquire()
        # read the header info from the memory map
        # go to beginning of file. 
        self.Mapfile.seek(0)
        self.inb = self.Mapfile.read(self.memory.size)
        #
        # The inheriting class should pick up from this point in the
        # data stream.
        #
        if (self.debug):
            print('Releasing the semaphore.')
        self.semaphore.release()

        #
        # Unpack the header information.
        #
        self.Length                  = self.Unpack('l')
        self.LastUpdate_Time_tv_sec  = self.Unpack('l')
        self.LastUpdate_Time_tv_nsec = self.Unpack('l')
        self.DoubleData              = self.Unpack('d')
        self.LAM                     = self.Unpack('l')

        #
        # Update this if the data is new.
        #
        if (self.debug):
            print ('Update: ', self.TimeSinceLastUpdate())
        if self.TimeSinceLastUpdate() != 0.0:
            self.LastRead_Time_tv_sec  = self.LastUpdate_Time_tv_sec
            self.LastRead_Time_tv_nsec = self.LastUpdate_Time_tv_nsec
            #print ('Update: ', self.TimeSinceLastUpdate())

        if (self.debug):
            print ("bytes used: ", self.bytes)
            print ("Length: ", self.Length)
            print ("Sec: ",    self.LastUpdate_Time_tv_sec)
            print ("nsec: ",   self.LastUpdate_Time_tv_nsec)
            print ("double: ", self.DoubleData)
            print ("bool: ",   self.LAM)
            print ('SharedMem2 debug end ============================ ')
        #time.sleep(1)

    def TimeSinceLastUpdate(self):
        """
        Returns the time in seconds in how old the data is.
        """
        Dns = self.LastUpdate_Time_tv_nsec - self.LastRead_Time_tv_nsec
        Ds  = self.LastUpdate_Time_tv_sec - self.LastRead_Time_tv_sec
        return (Ds + Ds/1.0e9)
    
    def DataAge(self):
        """
        returns the age of data relative to the current clock time.
        https://docs.python.org/3/library/time.html
        """
        Dns = time.clock_gettime_ns(time.CLOCK_HIGHRES) - self.LastUpdate_Time_tv_nsec
        Ds  = time.clock_gettime(time.CLOCK_HIGHRES) - self.LastUpdate_Time_tv_sec
        return (Ds + Ds/1.0e9)


    def Write(self, value):
        """
        Write a string value to a shared memory segment
        that has been defined by the C program.
        """
        # set this up for debug. 
        print('WRITE: ', value)
