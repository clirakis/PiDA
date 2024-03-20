##################################################################
#
#	Makefile for Timeing using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	17-Mar-24      CBL     Original
#
#
######################################################################
# Machine specific stuff
#
#
TARGET = Timing
#
# Compile time resolution.
#
INCLUDE = -I$(DRIVE)/common/utility -I$(DRIVE)/common/RT_Tools \
	-I/usr/include/hdf5/serial
LIBS = -lutility -lRT_tools -lio -lhdf5_cpp -lhdf5
LIBS += -L$(HDF5LIB) -lconfig++


# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp Timing.cpp UserSignals.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Timing.hh UserSignals.hh Version.hh

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
