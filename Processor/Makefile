##################################################################
#
#	Makefile for Processor using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	23-Feb-22       CBL     Original
#
#
######################################################################
# Machine specific stuff
#
#
TARGET = Processor

NMEA_GPS = ../GTOP
IMU      = ../ICM-20948
COMMON   = $(HOME)/common
#
# Compile time resolution.
#
INCLUDE = -I$(COMMON)/utility -I$(COMMON)/iolib -I$(COMMON)/libNavBasic \
	-I$(NMEA_GPS) -I$(IMU) -I/usr/include/hdf5/serial

LIBS = -lNMEA -lIMUData -lio -lutility -lNavBasic -lproj -lhdf5_cpp -lhdf5
LIBS += -L$(IMU) -L$(NMEA_GPS) -L$(DRIVE)/common/iolib -L$(HDF5LIB) -lconfig++

# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp Processor.cpp UserSignals.cpp smIPC_GPS.cpp smIPC_IMU.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Processor.hh UserSignals.hh smIPC_GPS.hh smIPC_IMU.hh Version.hh

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
