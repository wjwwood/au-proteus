#!/usr/bin/env python
# encoding: utf-8

"""
proteus.py - Contains the Proteus object that provides an interface to the proteus robot.

Created by William Woodall on 2010-04-20.

Modified by Justin Paladino 2010-07-01
"""
__author__ = "William Woodall" 

###  Imports  ###

# Python libraries
import sys
import math
import thread
import time
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
OP_LEDS			= '\x68' # h
OP_FULL_MODE    = '\x65' # e

SENSOR_ODOM     = '\x01' # b00000001
SENSOR_IR       = '\x02' # b00000010
SENSOR_SERVO    = '\x06' # b00000110
SENSOR_COMPASS  = '\x04' # b00000100

START_CMD     = CMD_START+OP_START+CMD_STOP
STOP_CMD      = CMD_START+OP_STOP+CMD_STOP
SAFE_MODE_CMD = CMD_START+OP_SAFE_MODE+CMD_STOP
FULL_MODE_CMD = CMD_START+OP_FULL_MODE+CMD_STOP

# Log system, this should be overriden with something like rospy.loginfo or rospy.logerr
#  I do this in an effort to remove all ros dependant code from this file

logfile = open('proteus-log.txt', 'a')
def loginfo(msg):
    print >> logfile,  msg
    
def logerr(msg):
    print >> logfile, "Err: " + msg

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
        self.serial.timeout = 1
        self.serial.open()
        
        # Setup some variables
        self.started = False
        
        self.onOdomData = None
        self.odom_timer = None
        
        self.servo_timer = None
        self.onServoData = None
        
        self.onIRSensorData = None
        self.ir_timer = None
        
        self.running = True
        self.com_lock = Lock()
        
        self.serial_reader = thread.start_new_thread(self.readSerial, (self.serial,))
        
    def stopCS(self):
        self.running = True
        self.serial_reader = thread.start_new_thread(self.readSerial, (self.serial,))
        
    def startCS(self):
        self.running = False
        
    def close(self):
        self.running = False
        self.serial.close()
    
    def pollIRSensors(self):
        """Polls the IR Sensors on a regular period
            returns a list of six IR readings in milimeters
        """
        if not self.started:
            print "Start the Proteus and try again."
            return
        else:
            self.ir_timer = Timer(self.ir_poll_rate, self.pollIRSensors) # Kick off the next timer
            self.ir_timer.start()
        ir_data = [0,0,0,0,0,0]
        data = None
        try:          
            self.startCS() #Critical Section
            # Request for the IR data
            self.write(CMD_START+OP_SENSOR+SENSOR_IR+CMD_STOP)
            # Wait for the proper response
            data = self.read(12)
            self.stopCS() #End Critical Section
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
                
    def testServo(self, low, high):
        """ Takes the lower and higher limits for a test of servo range
            in the form of: testServo(x, y)
            will loop through the values and effectively call the function
            self.move(0,0.x) -> self.move(0, 0.y) and values in between"""
        
        for i in range(-148,420):
            if ((i % 4) == 0):
                temp = i / 1000.0 # I figured out that this should cover all the unique return values.
                self.move(0,temp)
                print temp
                self.readOdom()
                time.sleep(0.2)
                
                
    def pollOdom(self):
        """Polls the odometry on a regular period
            returns - [tach, steering angle, motor stall]
            tach - is in meters
            steering angle - is in radians
            motor stall - is True, False, or None if no data is available
        """
        if not self.started:
            print "Start the Proteus and try again."
            return
        else:
            self.odom_timer = Timer(self.odom_poll_rate, self.pollOdom) # Kick off the next timer
            self.odom_timer.start()
        odom_data = [0,0,None]
        data = None
        try:
            self.startCS() #Critical Section
            # Request for the IR data
            self.write(CMD_START+OP_SENSOR+SENSOR_ODOM+CMD_STOP)
            # Wait for the proper response
            data = self.read(5)
            self.stopCS() #End Critical Section
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
                
    def readOdom(self):
        """ Reads the IR data once
            returns - [tach, steering angle, motor stall]
            tach - is in meters
            steering angle - is in degrees
            motor stall - is True, False, or None if no data is available
        """
        if not self.started:
            print "Start the Proteus and try again."
            return
        elif self.serial.isOpen():
            odom_data = [0,0,None]
            data = None
            self.startCS() #Critical Section
            self.write(CMD_START+OP_SENSOR+SENSOR_ODOM+CMD_STOP)
            data = self.read(5)
            self.stopCS() # End Critical Section
            
            # Parse the data
            if data != None and len(data) == 5:
                # Encode the data as HEX for processing
                data = data.encode("HEX")
                # Extract the Tach
                odom_data[0] = (int(data[0:4], 16)) * 0.0012833
                temp = (int(data[4:8], 16)) * 0.0001
                temp = temp / 3.14159
                temp *= 180.0
                if temp > 180.0:
                    temp = temp - 360.0
                if temp == -8.25: temp = 0.0
                odom_data[1] = temp
                temp = (int(data[8:10], 16))
                if temp != None:
                    if temp > 0:
                        odom_data[2] = True
                    elif temp == 0:
                        odom_data[2] = False
                else:
                    odom_data[2] = None
            print odom_data
        else:
            logerr('Error: Serial port not open')
    
        
    def start(self):
        """Sets the proteus in the start mode, starts polling sensors"""
           
        self.started = True
        # Start any polling threads
        
        # Send start cmd to Proteus
        if self.serial.isOpen():
            self.write(START_CMD)
            self.write(SAFE_MODE_CMD)
        else:
            logerr('Error: Serial port not open')
			
    def safe(self):
        """Sets the proteus in safe mode, making sure that drive commands come regularly"""
        if self.started == False:
            print "Start the Proteus and try again."
        elif self.serial.isOpen():
            self.write(SAFE_MODE_CMD)
        else:
            logerr('Error: Serial port not open')
            
    def full(self):
        """Sets the proteus in full mode, the motor might get stuck on."""
        if self.started == False:
            print "Start the Proteus and try again."
        elif self.serial.isOpen():
            self.write(FULL_MODE_CMD)
        else:
            logerr('Error: Serial port not open')
        
    def read(self, value):
        "Read X number of values from the com with a lock."
        with self.com_lock:
            temp = self.serial.read(value)
            if temp: loginfo("Read: " + temp)
            #log temp to file
            return temp
            
    def readline(self):
        "Reads a line from the com with a lock"
        with self.com_lock:
            temp = self.serial.readline()
            if temp: loginfo("Readline: " + temp)
            return temp
            
    def write(self, value):
        "Write's to the com with a lock"
        with self.com_lock:
            loginfo("Writing: " + value)
            self.serial.write(value)

    def stop(self):
        """Sets the proteus into stop mode cleans up, joins threads"""
    
        self.move(0,0) # Stop the motors
        self.started = False
        # Stop any polling threads
        
        # Send stop cmd to Proteus
        if self.serial.isOpen():
            self.write(STOP_CMD)
        else:
            logerr('Error: Serial port not open')
        # Join Threads
		
    def led(self, which):
        "TODO: doc for led"
        if which < 0:
            temp = abs(int(which))
            temp = temp - 1
            temp = ~temp
            temp = 0x80 | temp
        else:
            temp = which		
        cmd = ""
        cmd += CMD_START
        cmd += OP_LEDS
        cmd += chr(temp & 0xFF)
        cmd += CMD_STOP
        if self.serial.isOpen():
            self.write(cmd)
        else:
            logerr('Error: Serial port not open')
        
    def pollServo(self):
        "TODO: Doc for pollServo"
        cmd = ""
        cmd += CMD_START
        cmd += OP_SENSOR
        cmd += SENSOR_SERVO
        cmd += CMD_STOP
        self.startCS() #Critical Section
        if self.serial.isOpen():
            self.write(cmd)
        else:
            logerr('Error: Serial port not open')
        data = self.read(2)
        self.stopCS() # End Critical Section
        data = data.encode("HEX")
        servo_data = int(data[0:4], 16)
        print servo_data
    
    def move(self, speed, direction):
        """Translates speed and direction into commands the proteus understands and sends them"""
    
        # If the Proteus isn't started ignore
        if not self.started:
            print "Start the Proteus and try again."
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
            direction = 375 + direction*45 # direction should be negative
        else:
            direction *= 45
        direction = (direction * 3.14159) / 180
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
            self.write(cmd)

    def readSerial(self, serial):
        "Read's all com activity"
        while self.running:
            temp = self.readline()
            if temp != "":
                print temp
        print "Read Serial Done"

