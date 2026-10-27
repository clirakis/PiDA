#!/usr/bin/env python
 # ================================================================
 #
 # Module Name : OPT.py
 #
 # Author/Date : C.B. Lirakis / 26-Oct-26
 #
 # Description : python interface to qwiic directly, there appears to
 #               be no library for accessing the 4048
 #
 #
 # Restrictions/Limitations : 
 #
 # Change Descriptions :
 #
 # Classification : Unclassified
 #
 # References :
 #    More information on Qwiic is at https://www.sparkfun.com/qwiic
 #    library  documentation -- https://github.com/sparkfun/Qwiic_Py
 #    API reference - https://qwiic-py.readthedocs.io/en/latest/apiref.html#qwiic
 #
 # Dependencies:
 #    Since this is python it is run in a env -- .envSparkfun
 #    This container has the qwiic and qwiic_ads1015 (ADC) libraries
 #    installed. 
 #
 #    Modified    By    Reason
 #    --------    --    ------
 #
 # ================================================================

# Imports!
import qwiic
import sys
import time
import datetime
import argparse



if __name__ == '__main__':

    # default number of samples
    N = 10
    # default elapsed time, hard coded to 1/sec so this ends up
    # being the number of samples.

    # Default gain
    Gain = 4

    parser = argparse.ArgumentParser("Incident UV tool.")
    parser.add_argument("-n", "--number", help="Number samples")
    parser.add_argument("-g", "--gain",   help="ADC amplifier gain.")

    args = parser.parse_args()
    
#    if len(sys.argv)>1:
#        N = int(sys.argv[1])

    try:
        uv = UVSensor(int(args.gain))
        uv.Do(int(args.number))
        
    except (KeyboardInterrupt, SystemExit) as exErr:
        print("\nEnding UV")
        sys.exit(0)
