package com.GCS.xbee;

public class PlaneData {
	
	public int currLat;
	public int currLng;
	public int currAlt;
	public int nextLat;
	public int nextLng;
	public int nextAlt;
	public int ground_speed;
	public int target_bearing;
	public int currWP;
	public int WPdistance;
	
	public String toString() {
		return currLat + " " + currLng + " " + currAlt + " " + nextLat + " " + nextLng + " " +
		nextAlt + " " + ground_speed + " " + target_bearing + " " + currWP + " " + WPdistance;
	}
	
}
