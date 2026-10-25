#!/usr/bin/env python
 # ================================================================
 #
 # Module Name : UV.py
 #
 # Author/Date : C.B. Lirakis / 25-Oct-26
 #
 # Description : python interface to qwiic 12 bit adc and subsequently
 #               connected to a UV sensor.
 #
 #             At gain setting of 1, like in this example (and 3.3V VCC),
 #             0-3.3V will read 0-1652.
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
 #    API -- https://qwiic-py.readthedocs.io/en/latest/apiref.html#qwiic
 #    hardware for ADS1015 -- https://github.com/sparkfun/SparkFun_Qwiic_12_Bit_ADC_-_4_Channel_ADS1015
 #    API for ADS1015 -- https://github.com/sparkfun/qwiic_ads1015_py
 #
 # Dependencies:
 #    Since this is python it is run in a env -- .envSparkfun
 #    This container has the qwiic and qwiic_ads1015 (ADC) libraries
 #    installed. 
 #
 #    Modified    By    Reason
 #    --------    --    ------
 #    
 #
 # ================================================================

# Imports!
import qwiic_ads1015
import sys
import time
import datetime

class UVSensor:
    def __init__(self):
        """
        Initialize the system. This means opening the Qwiic port
        """
        self.__error__ = 0 # no error
        self.__debug__ = False
        now = datetime.datetime.now()
        self.logfile = open('UV.log','w') 
        self.logfile.write("# ---------------------------------------------\n")
        self.logfile.write("# SYS-I-FILE: UV sensor \n");
        self.logfile.write('# SYS-I-FILE: Logfile started: '+str(now)+'\n')

        # Attach to subsystem
        self.myAds = qwiic_ads1015.QwiicADS1015()
        if self.myAds.is_connected() == False:
            print("The device isn't connected to the system. Please check your connection", file=sys.stderr)
            self.__error__ = 1 # not good.
            self.logfile.write("# ERROR opening the qwiic bus.\n")
            return
    
        self.myAds.begin()
        # set the gain. 
        self.myAds.set_gain(self.myAds.kConfigPga1)

        # This is the multiplier for converting the UV
        # value, V0 = 4.3 * Diode current in uA.
        # 1uA = 9mW/cm^2
        # inverting this. then it is 9/4.3 or
        # ~2.09
        # This is assuming that it is a +5 Vcc with 4.3V as maximum.
        # not sure if this is the full story and max on this board
        # may be 3.3 volts. 
        self.UV_Mult = 9/4.3
        self.Count   = 0
        self.Run     = True
        self.logfile.write("# Count, Seconds, mv, UV mw/cm^2\n")
        
    def Do(self, Niterations):
        """
        Read the data. Currently the UV sensor is on channel 3
        so only the data at channel 3 need be sampled.
        @param Niterations - number of iterations to run, 0 means
               indefinite. 
        """
        while self.Run:
            channel_A3 = self.myAds.get_single_ended(3)
	    if self.__debug__:
	        print("A3: ", channel_A3, end = '\t')
        
            # Get the multiplier for the gain setting
            multiplier = self.myAds.get_multiplier()
            # The private variable _multiplierToVolts
            # is auto-updated each time setGain is called
            mV = channel_A3 * multiplier
            UV = mV*self.UV_Mult*1.0e-3
	    if self.__debug__:
                print("result: ", mV, "mV, UV:", UV, " mW/cm2")

            toWrite = str(self.Count) + "," + time.time() + "," + str(mV) + "," + str(UV) + "\n"
            self.logfile.write( toWrite)

            self.Count = self.Count + 1
            if Niterations>0:
                self.Run = (self.Count<Niterations)
        
            time.sleep(0.10)


    def __del__(self):
        """
        Do any cleanup necessary. 
        """
        self.logfile.write('# SYS-I-END: Program ends.\n')
        now = datetime.datetime.now()
        self.logfile.write('# SYS-I-ENDTIME ' + str(now) + '\n')
        self.logfile.close()


if __name__ == '__main__':

    try:
        uv = UVSensor()
        uv.Do(100)
        
    except (KeyboardInterrupt, SystemExit) as exErr:
        print("\nEnding UV")
        sys.exit(0)
