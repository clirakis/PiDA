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
 # Notes:
 #  4 sensors
 #      Channel 0 X - Red     - Peak 600nm
 #      Channel 1 Y - Green   - peak 550nm
 #      Channel 2 Z - Blue    - peak 460nm
 #      Channel 3 W - White   - peak 500nm
 #
 #  Conversion Time = Register 0xB) - this may be a way
 #                          to send an onboard hardware interrupt
 #      0  600us
 #      1   1.0ms
 #      2   1.8ms
 #      3   3.4ms
 #      4   6.5ms
 #      5  12.7ms
 #      6  25.0ms
 #      7  50.0ms
 #      8 100.0ms
 #      9 200.0ms
 #     10 400.0ms
 #     11 800.0ms
 #
 #
 #  Range 0xC    - look for light resolution table 8.5 
 #      0   2254 lux
 #      1   4509 lux
 #      2   9018 lux
 #      3  18036 lux
 #      4  36071 lux
 #      5  72142 lux
 #      6 144248 lux
 #     12 AUTOMATIC lux
 #
 #  Output Sample counter COUNTER_CHx
 #
 # Data read out
 #   MANTISSA_CHx  = (RESULT_MSB_CHx<<8) + RESULT_LSB_CHx
 #   ADC_CODES_CHx = (MANTISSA_CHx<<EXPONENT_CHx) 
 #
 # Look for Register Write Operation figure 8.6
 # What does I2C_BURST mean?
 #
 # Register Map
 # AD  D15| D14| D13| D12| D11| D10| D09| D08| D07| D06| D05|D04| D03| D02 |D01| D00
 # 00h|EXP_CH0(D15:D12) MSB_CH0(D11:D0)
 # 01h LSB_CH0(D15:D8) COUNTER_CH0(D7:D0)
 #
 # 02h|EXP_CH1(D15:D12) MSB_CH1(D11:D0)
 # 03h LSB_CH1(D15:D8) COUNTER_CH1(D7:D0)
 #
 # 04h|EXP_CH2(D15:D12) MSB_CH2(D11:D0)
 # 05h LSB_CH2(D15:D8) COUNTER_CH2(D7:D0)
 #
 # 06h|EXP_CH3(D15:D12) MSB_CH3(D11:D0)
 # 07h LSB_CH3(D15:D8) COUNTER_CH3(D7:D0)
 #
 # 08h|THRESHOLD_L_EXP(D15:D12) THRESHOLD_L_RESULT(D11:D0)
 # 09h|THRESHOLD_H_EXP(D15:D12) THRESHOLD_H_RESULT(D11:D0)
 # 0Ah QWAKE(D15) 0(D14) RANGE(D13:D10) CONVERSION_TIME(D9:D6) OPERATING_MODE(D6:D4) LATCH(D3) INT_POL(D2) FAULT_COUNT(D1:D0)
 #
 # 0Bh 128 (D15:D7) THRESH_CH_SEL(D6:D5) INT_DIR(D4) INT_CFG(D3:D2) 0(D1) I2C_BURST(D0)
 #
 # 0Ch 0(D15:D4) OVERLOAD_FLAG(D3) CONVERSION_READY(D2) FLAG_H(D1) FLAG_L(D0)
 # 11h 0(D15:D14) DIDL(D13:D12) DIDH(D11:D0)
 # ================================================================

# Imports!
from smbus2 import SMBus
import sys
import time
import datetime
import argparse
import struct

kBUS = 1
kADDRESS = 0x44

class OPT4048:
    def __init__(self):
        self.bus = SMBus(kBUS)
        self.myDebug = True

    def ReadRegister(self, rAddress):
        """
        read the 16 bit register and do the byte order corrections.
        @param - rAddress is the register address as shown above.
        @return - Fully constructed 16 bit word
        """
        # Get two bytes of data then form the word. 
        data = self.bus.read_i2c_block_data(kADDRESS, rAddress, 2)

        value = (data[0]<<8) | data[1]
        if self.myDebug:
            print('------------------ RAW -----------------------------')
            print(f'Read register: 0x{rAddress:x}, value: 0x{value:x}')
        return value

    def WriteRegister(self, rAddress, Value):
        """
        Pack the data in Value in the correct word order and
        write it to the register given in rAddress
        @param rAddress - register address to write to.
        @param Value    - 16 bit value to write to the register.
        """
        # Ensure input is only 16 bits.
        wordData = Value&0xFFFF
#        data[0] = (wordData>>8)&0xFF
#        data[1] = wordData&0xFF
        data = struct.pack('BB', (wordData>>8)&0xFF, wordData&0xFF)
        print(f'PACKED WRITE: 0x{data,x}')
        self.bus.write_i2c_block_data(kADDRESS, rAddress, data)
        

    def ReadChannel(self, channel):
        """
        read the two words of data for the channel specified and
        construct the value.
        @param channel - channel to read {0:3}
        @return - value of that channel
        """
        rv = -1
        if channel<4:
            address = channel+2
            Upper = self.ReadRegister(address)
            Lower = self.ReadRegister(address+1)
            Exponent = (Upper>>4) & 0X0F
            MSB      = Upper & 0x0F
            LSB      = (Lower >>8) & 0xFF
            COUNTER  = (Lower>>4)  & 0x0F
            CRC      = Lower&0x0F
            
            #   MANTISSA_CHx  = (RESULT_MSB_CHx<<8) + RESULT_LSB_CHx
            #   ADC_CODES_CHx = (MANTISSA_CHx<<EXPONENT_CHx) 
            rv = ((MSB<<8) | LSB)<<Exponent
            if self.myDebug:
                print('-----------------------------------------------')
                print(f'channel: {channel}, value: {rv}, count {COUNTER}')
                print(f'  {Upper:x}, {Lower:x}, {Exponent}, {MSB}, {LSB}')
                print(f'  {COUNTER}, {CRC:x}')

        return rv

    def ThresholdL(self):
        """
        read theshold data low
        """
        value = self.ReadRegister(0x8)
        Exponent = (value>>4) & 0xF
        Mant     = value&0xF
        rv = Mant<<Exponent
        if self.myDebug:
            print('-----------------------------------------------')
            print(f'Threshold Low: {rv:x}')
        return rv

    def ThresholdH(self):
        """
        read theshold data low
        """
        value = self.ReadRegister(0x9)
        Exponent = (value>>4) & 0xF
        Mant     = value&0xF
        rv = Mant<<Exponent
        if self.myDebug:
            print('-----------------------------------------------')
            print(f'Threshold High: {rv:x}')
        return rv

    def Status(self):
        """
        Read the status of the device.
        """
        value = self.ReadRegister(0xA)
        QWAKE = value>>15
        RANGE = (value>>10)&0xF
        CONV  = (value>>6)&0xF
        MODE  = (value>>4)&0x3
        LATCH = (value>>3)&0x1
        INT_POL = (value>>2)&0x1
        FAULT = value&0x3
        if self.myDebug:
            print('-----------------------------------------------')
            print(f'Status, QWAKE: {QWAKE}, RANGE: {RANGE}, CONVERSION_TIME {CONV}')
            print(f'Mode: {MODE}, Latch{LATCH}, Interrupt Polarity: {INT_POL}')
            print(f'Fault: {FAULT}')

    def Trigger(self):
        """
        Read the trigger setup.
        """
        value = self.ReadRegister(0xB)

        High = value>>7
        CH_SEL  = (value>>5)&0x3
        INT_DIR = (value>>4)&0x1
        INT_CFG = (value>>2)&0x3
        BURST   = value&0x1

        if self.myDebug:
            print('-----------------------------------------------')
            print(f'Trigger, High: {High}, Channel select: {CH_SEL}')
            print(f'Interrupt direction: {INT_DIR}, configuration: {INT_CFG}')
            print(f'I2C burst: {BURST}')
            
    def CStatus(self):
        """
        read the configuration status
        """
        value = self.ReadRegister(0xC)

        High = value>>4
        OVER  = (value>>3)&0x1
        READY = (value>>2)&0x1
        FLAGS = value&0x3

        if self.myDebug:
            print('-----------------------------------------------')
            print(f'CSTATUS, High: {High}, Overload flag: {OVER}')
            print(f'    Ready: {READY}, Flags:{FLAGS:x}')

    def ReadDIDL(self):
        """
        Device ID
        """
        value = self.ReadRegister(0x11)

        DIDL = (value>>12)&0x3
        DIDH = value&0xFFF

        if self.myDebug:
            print('-----------------------------------------------')
            print(f'DIDL 0x:{DIDL:x}, DIDH 0x:{DIDH:x}')
        

    def WriteSetup(self):
        """
        Register 0xA does setup. 
        """
        # For the moment hand pack the data.
        Value = 0x3238
        self.WriteRegister(0x0A, Value)
        
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
        opt = OPT4048()
        opt.ThresholdL()
        opt.ThresholdH()
        opt.Status()
        opt.Trigger()
        opt.CStatus()
        opt.ReadDIDL()
        #
        print('Write Setup and then read back.')
        opt.WriteSetup()
        opt.ReadChannel(0)
        opt.ReadChannel(1)
        opt.ReadChannel(2)
        opt.ReadChannel(3)
        
    except (KeyboardInterrupt, SystemExit) as exErr:
        print("\nEnding UV")
        sys.exit(0)
