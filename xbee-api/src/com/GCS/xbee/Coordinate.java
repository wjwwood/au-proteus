package com.GCS.xbee;

/**
 * A 3-D coordinate used by the collision avoidance algorithm for calculations and the XBeeGCS for transmitting packets.
 * Note: while Coordinate fields are doubles, they should be large enough numbers (i.e. > 1 million)
 * so the ArduPilot can use them.
 */
public class Coordinate {
	double x;
	double y;
	double z;

	public String toString() { return "<" + x + "," + y + "," + z + ">"; }
}
