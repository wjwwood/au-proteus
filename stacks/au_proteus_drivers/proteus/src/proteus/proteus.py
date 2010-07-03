#!/usr/bin/env python
# encoding: utf-8

"""
proteus.py - Contains the Proteus object that provides an interface to the proteus robot.

Created by William Woodall on 2010-04-20.
"""
__author__ = "William Woodall"

###  Imports  ###

# Python libraries
import sys
import math
from threading import Lock, Timer

# pySerial
from serial import Serial

# Peer libraries
from logerror import logError

###  Defines  ###

CMD_START       = '\x24' # $
CMD_STOP        = '\x0A' # LF

OP_START        = '\x61' # a
OP_STOP         = '\x66' # f
OP_DRIVE        = '\x67' # g
OP_SAFE_MODE    = '\x64' # d
OP_SENSOR       = '\x69' # i

SENSOR_ODOM     = '\x01' # b00000001
SENSOR_IR       = '\x02' # b00000010
SENSOR_COMPASS  = '\x04' # b00000100

START_CMD     = CMD_START+OP_START+CMD_STOP
STOP_CMD      = CMD_START+OP_STOP+CMD_STOP
SAFE_MODE_CMD = CMD_START+OP_SAFE_MODE+CMD_STOP

# Log system, this should be overriden with something like rospy.loginfo or rospy.logerr
#  I do this in an effort to remove all ros dependant code from this file
def loginfo(msg):
    print msg
    
def logerr(msg):
    print >> sys.stderr, msg

###  Classes  ###
class Proteus(object):
    """This class allows you to control a Proteus robot"""
    def __init__(self, serial_port=None, ir_poll_rate=10, odom_poll_rate=10):
        """Constructor"""
        # Use the passed parameters or the defaults
        self.serial_port = serial_port or "/dev/ttyS0"
        self.ir_poll_rate = 1.0/ir_poll_rate # Convert Hz to period
        self.odom_poll_rate = 1.0/odom_poll_rate # Convert Hz to period
        # Create and setup the serial port
        self.serial = Serial()
        self.serial.port = self.serial_port
        self.serial.baudrate = 57600
        self.serial.open()
        
        # Setup some variables
        self.started = False
        
        self.sensor_lock = Lock()
        
        self.onOdomData = None
        self.odom_timer = None
        
        self.onIRSensorData = None
        self.ir_timer = None
    
    def pollIRSensors(self):
        """Polls the IR Sensors on a regular period
            returns a list of six IR readings in milimeters
        """
        if not self.started:
            return
        else:
            self.ir_timer = Timer(self.ir_poll_rate, self.pollIRSensors) # Kick off the next timer
            self.ir_timer.start()
        ir_data = [0,0,0,0,0,0]
        data = None
        try:
            # Acquire the sensor lock to prevent sensors from polling at the same time
            self.sensor_lock.acquire()
            # Request for the IR data
            self.serial.write(CMD_START+OP_SENSOR+SENSOR_IR+CMD_STOP)
            # Wait for the proper response
            data = self.serial.read(12)
        except Exception as err:
            logError(sys.exc_info(), logerr, "Exception while polling IR Sensors:")
        finally:
            # Release the sensor lock
            self.sensor_lock.release()
        try:
            # Parse the data
            if data != None and len(data) == 12:
                # Encode the data as HEX for processing
                data = data.encode("HEX")
                for ir in range(0,6):
                    ir_data[ir] = int(data[(ir*4):((ir+1)*4)], 16)
                # Here is where you can convert the data back to ADC and then into other IR sensor specific distances
                pass
        except Exception as err:
            logError(sys.exc_info(), logerr, "Exception while polling IR Sensors:")
        finally:
            # Pass the data along to the handler
            if self.onIRSensorData:
                self.onIRSensorData(ir_data)
    
    def pollOdom(self):
        """Polls the odometry on a regular period
            returns - [tach, steering angle, motor stall]
            tach - is in meters
            steering angle - is in radians
            motor stall - is True, False, or None if no data is available
        """
        if not self.started:
            return
        else:
            self.odom_timer = Timer(self.odom_poll_rate, self.pollOdom) # Kick off the next timer
            self.odom_timer.start()
        odom_data = [0,0,None]
        data = None
        try:
            # Acquire the sensor lock to prevent sensors from polling at the same time
            self.sensor_lock.acquire()
            # Request for the IR data
            self.serial.write(CMD_START+OP_SENSOR+SENSOR_ODOM+CMD_STOP)
            # Wait for the proper response
            data = self.serial.read(5)
        except Exception as err:
            logError(sys.exc_info(), logerr, "Exception while polling Odometry:")
        finally:
            # Release the sensor lock
            self.sensor_lock.release()
        try:
            # Parse the data
            if data != None and len(data) == 5:
                # Encode the data as HEX for processing
                data = data.encode("HEX")
                # Extract the Tach
                odom_data[0] = (int(data[0:4], 16)) * 0.0012833
        except Exception as err:
            logError(sys.exc_info(), logerr, "Exception while polling Odometry:")
        finally:
            # Pass the data along to the handler
            if self.onOdomData:
                self.onOdomData(odom_data)
    
    def start(self):
        """Sets the proteus in the start mode, starts polling sensors"""
        self.started = True
        # Start any polling threads
        
        # Send start cmd to Proteus
        if self.serial.isOpen():
            self.serial.write(START_CMD)
            self.serial.write(SAFE_MODE_CMD)
        else:
            logerr('Error: Serial port not open')
    
    def stop(self):
        """Sets the proteus into stop mode cleans up, joins threads"""
        self.move(0,0) # Stop the motors
        self.started = False
        # Stop any polling threads
        
        # Send stop cmd to Proteus
        if self.serial.isOpen():
            self.serial.write(STOP_CMD)
        else:
            logerr('Error: Serial port not open')
        # Join Threads
        
    
    def move(self, speed, direction):
        """Translates speed and direction into commands the proteus understands and sends them"""
        # If the Proteus isn't started ignore
        if not self.started:
            return
        # Convert the speed into a velocity the Proteus can use
        if speed > 1.0:
            speed = 1.0
        elif speed < -1.0:
            speed = -1.0
        speed *= 100
        speed = int(speed) & 0x0000ffff
        speed_1 = speed >> 8
        speed_2 = speed & 0x00ff
        proteus_speed = chr(speed_1)+chr(speed_2)
        # Convert the direction into an angle the Proteus can turn to
        if direction > 1.0:
            direction = 1.0
        elif direction < -1.0:
            direction = -1.0
        if direction < 0:
            direction = 300 - direction*60 # direction should be negative so you're actually adding to 300
        else:
            direction *= 60
        direction = math.radians(direction)
        direction *= 10000
        direction = int(direction)
        dir_1 = direction >> 8
        dir_2 = direction & 0x00ff
        proteus_direction = chr(dir_1)+chr(dir_2)
        # Construct the command
        cmd = ""
        cmd += CMD_START
        cmd += OP_DRIVE
        cmd += proteus_speed
        cmd += proteus_direction
        cmd += CMD_STOP
        # Send the command
        if self.serial.isOpen():
            self.serial.write(cmd)
    
