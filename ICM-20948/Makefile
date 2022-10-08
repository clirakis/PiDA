##################################################################
#
#	Makefile for IMU using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	25-Feb-22       CBL     Original
#
#
######################################################################
# Machine specific stuff
#
#
TARGET = IMU
#
# Compile time resolution.
#
INCLUDE = -I$(DRIVE)/common/utility -I$(DRIVE)/common/iolib \
	-I/usr/include/hdf5/serial
LIBS = -lwiringPi -lIMUData -lio -lutility -lhdf5_cpp -lhdf5
LIBS += -L. -L$(HDF5LIB) -lconfig++


# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp ICM-20948.cpp IMU.cpp smIPC.cpp \
	UserSignals.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = ICM-20948.hh IMU.hh IMUData.hh smIPC.hh UserSignals.hh Version.hh

libIMUData.a: IMUData.cpp IMUData.hh
	ar -r libIMUData.a IMUData.o

# When we build all, what do we build?
all:      $(TARGET) libIMUData.a

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
