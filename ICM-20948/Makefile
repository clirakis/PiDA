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
INCLUDE = -I../GTOP -I$(DRIVE)/common/utility -I$(DRIVE)/common/iolib \
	-I/usr/include/hdf5/serial

LIBS = -L. -L../GTOP -L$(HDF5LIB) 
LIBS += -lNMEA -lIMUData -lio -lutility -lhdf5_cpp -lhdf5 -lconfig++


# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp ICM-20948.cpp IMU.cpp smIPC.cpp \
	UserSignals.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = ICM-20948.hh IMU.hh IMUData.hh smIPC.hh UserSignals.hh Version.hh


# When we build all, what do we build?
all:      $(TARGET) 

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
