##################################################################
#
#	Makefile for Timeing using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	17-Mar-24      CBL     Original
#       24-Mar-24      CBL     Added in GPS sm_IPC to get GPS timing data. 
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
INCLUDE = -I$(DRIVE)/common/utility -I$(DRIVE)/common/iolib \
	-I$(DRIVE)/common/RT_Tools \
	-I/usr/include/hdf5/serial -I../GTOP/
LIBS = -lutility -lRT_tools -lio -lhdf5_cpp -lhdf5 -L ../GTOP/ -lNMEA
LIBS += -L$(HDF5LIB) -lconfig++


# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp Timing.cpp UserSignals.cpp smIPC.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Timing.hh smIPC.hh UserSignals.hh Version.hh

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
