##################################################################
#
#	Makefile for Barometer using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	24-Apr-24       CBL     Original, Happy palindrome day
#
#
######################################################################
# Machine specific stuff
#
#
TARGET = Barometer
#
# Compile time resolution.
#
INCLUDE = -I../GTOP -I$(DRIVE)/common/utility -I$(DRIVE)/common/iolib \
	-I/usr/include/hdf5/serial
LIBS = -L../GTOP -lNMEA -lio -lutility -lhdf5_cpp -lhdf5
LIBS += -L$(HDF5LIB) -lconfig++


# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp Barometer.cpp smIPC.cpp UserSignals.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Barometer.hh smIPC.hh UserSignals.hh Version.hh

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
