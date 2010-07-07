package com.GCS.xbee;

/**
 * A 3-D coordinate used by the collision avoidance algorithm for calculations and the XBeeGCS for transmitting packets.
 */
class Coordinate {
	double x;
	double y;
	double z;

	public String toString() { return "<" + x + "," + y + "," + z + ">"; }
}
