##################################################################
#
#	Makefile for IMU using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	25-Feb-22       CBL     Original
#       29-Mar-24       CBL     moved all AK09916 (magnetic) to separate module
#                               ALSO made I2CHelper
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
SRCCPP  = main.cpp ICM-20948.cpp AK09916.cpp I2CHelper.cpp IMU.cpp smIPC.cpp \
	UserSignals.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = ICM-20948.hh AK09916.hh I2CHelper.hh IMU.hh IMUData.hh smIPC.hh \
	UserSignals.hh Version.hh


# When we build all, what do we build?
all:      $(TARGET) 

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
