#!/usr/bin/python3
"""@brief TestGraph.py - test the base graph class I made

16-Dec-23    CBL    Original

"""
import numpy as np
from Plotting.GraphData import GraphData as gd

MyGD = gd()
def DoGraph():
    n = 10
    fx = np.random.normal(2.0, 1.0, n)
    fy = np.random.normal(2.0, 1.0, n)

    for i in range(n):
        MyGD.AddPoint(fx[i], fy[i])
    
    print(MyGD)
    
if __name__ == "__main__":
    """
    This is our main entry point. I wonder if I could define all the
    classes here.
    """
    DoGraph()
