##################################################################
#
#	Makefile for GTOP using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	07-Jan-16       CBL     Original
#       19-Feb-22       CBL     Wrapper to add in dealing with config, 
#                               logfile etc.
#       20-Feb-22       CBL     Logfile enabled, smIPC enabled.  
#       23-Feb-22       CBL     v1.3 made library for consumer
#
#
######################################################################
# Machine specific stuff
#
#
TARGET = GTOP
#
# Compile time resolution.
#
INCLUDE = -I$(DRIVE)/common/utility -I$(DRIVE)/common/iolib \
	-I/usr/include/hdf5/serial



#HDF5LIB setup as part of shell file. 
#
LIBS = -lNMEA -lutility -lio  -lrt -lcurses -lhdf5_cpp -lhdf5
LIBS += -L./ -L$(HDF5LIB) -lconfig++

# Rules to make the object files depend on the sources.
SRC     = GTOP_utilities.c 
SRCCPP  = main.cpp GTOP.cpp GTOPdisp.cpp smIPC.cpp UserSignals.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = GTOP.hh GTOPdisp.hh GTOP_utilities.h smIPC.hh serial.h \
	 UserSignals.hh Version.hh


# When we build all, what do we build?
all:      $(TARGET) 

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
