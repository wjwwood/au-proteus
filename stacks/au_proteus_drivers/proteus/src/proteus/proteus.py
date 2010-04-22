#!/usr/bin/env python
# encoding: utf-8

"""
proteus.py - Contains the Proteus object that provides an interface to the proteus robot.

Created by William Woodall on 2010-04-20.
"""
__author__ = "William Woodall"

###  Imports  ###

# Python libraries
import math

# pySerial
from serial import Serial

# Peer libraries
from seriallistener import SerialListener
from logerror import logError

###  Defines  ###

CMD_START   = '\x24' # $
CMD_STOP    = '\x0A' # LF

OP_START    = '\x61' # a
OP_STOP     = '\x66' # f
OP_DRIVE    = '\x67' # g

START_CMD   = CMD_START+OP_START+CMD_STOP
STOP_CMD   = CMD_START+OP_STOP+CMD_STOP

###  Classes  ###
class Proteus(object):
    """This class allows you to control a Proteus robot"""
    def __init__(self, serial_port=None):
        """Constructor"""
        # Use the passed serial_port or the default
        self.serial_port = serial_port or "/dev/ttyS0"
        # Create and setup the serial port
        self.serial = Serial()
        self.serial.port = self.serial_port
        self.serial.baudrate = 57600
        self.serial.open()
        
        # Setup some variables
        self.onEncodersData = None
        self.onIRSensorData = None
        self.started = False
        
        # Setup the serial listener
        self.serial_listener = SerialListener(self.serial)
        
    def start(self):
        """Sets the proteus in the start mode, starts polling sensors"""
        self.started = True
        # Start the serial listener
        if not self.serial_listener.isListening():
            self.serial_listener.listen()
        # Start any polling threads
        
        # Send start cmd to Proteus
        self.serial.write(START_CMD)
    
    def stop(self):
        """Sets the proteus into stop mode cleans up, joins threads"""
        self.started = False
        # Stop the serial listener
        if self.serial_listener.isListening():
            self.serial_listener.stopListening()
        # Stop any polling threads
        
        # Send stop cmd to Proteus
        self.move(0,0) # Stop the motors too...
        self.serial.write(STOP_CMD)
        # Join Threads
        
    
    def move(self, speed, direction):
        """Translates speed and direction into commands the proteus understands and sends them"""
        # If the Proteus isn't started ignore
        if not self.started:
            return
        # Convert the speed into a velocity the Proteus can use
        if speed < 1.0:
            speed = 1.0
        elif speed > -1.0:
            speed = -1.0
        speed *= 180
        speed = int(speed) & 0x0000ffff
        speed_1 = speed >> 8
        speed_2 = speed & 0x00ff
        proteus_speed = chr(speed_1)+chr(speed_2)
        # Convert the direction into an angle the Proteus can turn to
        if direction < 1.0:
            direction = 1.0
        elif direction > -1.0:
            direction = -1.0
        if direction < 0:
            direction = 355 - direction*20
        else:
            direction *= 20
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
        self.serial.write(cmd)
    