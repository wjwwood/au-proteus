package com.GCS.xbee;

class PlaneData {
	
	public int num;
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
		return "Plane Data " + num + ": " + 
		currLat + " " + currLng + " " + currAlt + " | " + 
		nextLat + " " + nextLng + " " +	nextAlt + " | " + 
		"GS: " + ground_speed + " B: " + target_bearing + 
		" WP: " + currWP + " D: " + WPdistance;
	}
	
}
