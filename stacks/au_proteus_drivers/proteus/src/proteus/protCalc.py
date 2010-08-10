"""
protCalc.py - Contains a few useful things for calculations with proteus.

Created by Justin Paladino on 2010-08-10
"""
__author__ = "Justin Paladino" 

###  Imports  ###

# Python libraries
import math
    
def turningRadius(d, l, theta):
    """ Finds a turning radius for an Ackerman drive robot
        r = (d/2) + (l / sin(theta))
        
        r = turning radius
        l = center to center distance between front and rear wheels (any length unit)
        d = distance center to center between left and right wheels (also any length unit, use the same as l)
        theta = average of the left and the right wheel angles (returned by readOdom() function in proteus.py)
        
        Assumes Ackerman (Car-Like) steering
        
        Returns the turning radius in the units used for d and l """
    
    if theta != 0:
        r = (d/2) + (l / math.sin(math.radians(theta)))
        return r
    else:
        return 99999999 
        
def odomToCoord(z, r):
    """ Finds the (x,y) coordinates of the robot relative to where it was when 
        the previous odometry reading was taken.
        
        z = The distance traveled (reported by tach)
        r = turning radius (calculated by turningRadius()) 
        
        MAKE SURE THESE PARAMETERS ARE IN THE SAME UNITS
        
        Returns cartesian coordinates whose origin is at the beginning of the move/turn """
    
    temp = [0,0]
    
    theta = z/r
    phi = (3.14159 - theta) / 2
    c = 2 * r * (math.sin(theta / 2))
    x = c * math.cos(phi)
    y = c * math.sin(phi)
    temp[0] = x
    temp[1] = y
    
    return temp