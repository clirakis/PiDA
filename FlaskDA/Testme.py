#!/usr/bin/python3
from PySM.TSIPosition import TSIPosition
from PySM.TSIPVelocity import TSIPVelocity
from PySM.TSIPStatus import TSIPStatus
import logging
import matplotlib.pyplot as plt
import time
import random as rnd
import numpy as np
import time
#
# at the parent class level, create a logger.
# https://docs.python.org/3/library/logging.html#module-logging
#
logging.basicConfig(filename='Testme.log',level=logging.DEBUG)
logging.info('Testme program is starting.')
#
# .debug, .warning
MySM = TSIPosition()
if (MySM.NoError()):
    for i in range(4):
        MySM.Read()   # This causes "length exceeded"
        MySM.Print()
        #print(MySM)
        time.sleep(1)
else:
    print("ERROR: ", MySM.error)
    
##MySM = TSIPVelocity()
##if (MySM.NoError()):
##    MySM.Read()
##    MySM.Print()
##    print(MySM)

##MySM = TSIPStatus()
##if (MySM.NoError()):
##    MySM.Read()
##    MySM.Print()
##    print(MySM)
##else:
##    print("ERROR: ", MySM.error)

#xL = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100}
##xL = np.zeros(10)
##y  = np.zeros(10)
###seed = int(time.clock_gettime(time.CLOCK_REALTIME))
### Calling this with no arguments seeds the random number generator with
### the current time. 
##rnd.seed()
##mu = 50
##sigma = 15
##fig = plt.figure()
###plt.axis('on')
##ax  = fig.add_axes([0,0,1,1])
##for i in range(10):
##    xL[i] = i*10
##    y[i] = rnd.gauss(mu,sigma)
##    print('X: ', xL[i], ' Y:', y[i])
##ax.scatter(xL, y)
##ax.set_xlabel('monotonic')
##ax.set_ylabel('random gauss')
##ax.set_title('scatter plot')
##ax.axis('on')
##plt.show()
