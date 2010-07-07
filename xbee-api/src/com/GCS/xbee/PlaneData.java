package com.GCS.xbee;

/**
 * A "struct" containing fields for all of the telemetry data received from
 * the ArduPilot's print_position() function in GCS_XBee.pde
 */
public class PlaneData {
	
	/** Plane ID derived from order of first valid telemetry transmit */
	public int planeID;
	/** Current Latitude */
	public int currLat;			
	/** Current Longitude */
	public int currLng;			
	/** Current Altitude */
	public int currAlt;	
	/** Next waypoint's latitude */
	public int nextLat;		
	/** Next waypoint's longitude */
	public int nextLng;			
	/** Next waypoint's altitude */
	public int nextAlt;			
	/** Ground Speed measured by GPS */
	public int ground_speed;	
	/** Target Bearing calculated by GPS data */
	public int target_bearing;	
	/** Current waypoint index */
	public int currWP;			
	/** Distance to next waypoint */
	public int WPdistance;		
	
	@Override
	public String toString() {
		return "Plane Data " + planeID + ": " + 
		currLat + " " + currLng + " " + currAlt + " | " + 
		nextLat + " " + nextLng + " " +	nextAlt + " | " + 
		"GS: " + ground_speed + " B: " + target_bearing + 
		" WP: " + currWP + " D: " + WPdistance;
	}
	
}
