//float dlat,dlng;

void navigate()
{
	// do not navigate with corrupt data
	// ---------------------------------
	if (invalid_location || GPS_fix == BAD_GPS)
	{
		if(control_mode != CIRCLE){
			nav_roll = 300;  // Set up a gentle bank
			nav_pitch = 0;
		}
		return;
	}
	

	
//*********************************************************************************
//   Here we begin actual navigation computations
//
//     RYAN - I implement a computationally simple approximation to the full flow field navigation for WP tracking and loiter here
//            Basically Jason's approach with a few tweaks
//*********************************************************************************
	
	//  We only perform most nav computations if we have new gps data to work with
	if((GPS_update & GPS_POSITION) > 0){
	
		GPS_update ^= GPS_POSITION;
		
		//
		if((GPS_update & GPS_HEADING)>0){
			GPS_update ^= GPS_HEADING;
			ground_course_est = ground_course;
		}

		// Jason - I have rearranged this because there was a delay (till the next function call) in changing heading when reaching a waypoint

		// waypoint distance from plane
		// ----------------------------
		wp_distance = getDistance(&current_loc, &next_WP);          

	    // Are we there yet?
	    // wp_distance is in ACTUAL meters, not the *100 meters we get from the GPS
	    // ------------------------------------------------------------------------
	    if (control_mode == LOITER){
			// nothing to do really;
		
	    } else if (wp_distance < 0){
			//
			// something went wrong!!!
			// if our waypoints are too large we can get an wrapped number
			// -----------------------------------------------------------
			/*
			Serial.print("wp_distance error, loc: ");
			Serial.print(current_loc.lat);
			Serial.print(", ");
			Serial.print(current_loc.lng);
			Serial.print("  next_WP ");
			Serial.print(next_WP.lat);
			Serial.print(", ");
			Serial.println(next_WP.lng);
			*/
		
	    } else if (wp_distance < wp_radius) {  
			waypoint_event(EVENT_WILL_REACH_WAYPOINT);
			reached_waypoint();
			wp_distance = getDistance(&current_loc, &next_WP);    //Recalc distance for new waypoint			
	    }
		
		// target_bearing is where we should be heading 
		// --------------------------
		target_bearing 	= get_bearing(&current_loc, &next_WP);

		// nav_bearing is how we're actually going to get there
		// ----------------------------------------------------
		nav_bearing = target_bearing;
	
		if(control_mode == LOITER){
			float power;
			if (wp_distance <= LOITER_RADIUS){
				power = (float)wp_distance / (float)LOITER_RADIUS;
				//Serial.print("inside power ");
				//Serial.println((power*100),DEC);
				nav_bearing += 18000;   				//We loiter in a clockwise direction
				nav_bearing += power  * 9000;
			}else if (wp_distance < (LOITER_RADIUS*2)){
				power = (float)((LOITER_RADIUS*2) - wp_distance) / (float)LOITER_RADIUS;
				//Serial.print("outside power ");
				//Serial.println((power*100),DEC);
				nav_bearing += power * -9000;
			}
		} else {
			// Crosstrack Error
			// ----------------
			if (abs(target_bearing - crosstrack_bearing) < 4500) {   // If we are too far off or too close we don't do track following
				crosstrack_error = sin(radians((target_bearing - crosstrack_bearing)/100)) * wp_distance;   // Meters we are off track line
				nav_bearing += constrain(crosstrack_error * XTRACK_GAIN,-XTRACK_ENTRY_ANGLE,XTRACK_ENTRY_ANGLE);
	        }
		}
		

		// Wrap the values
		// ---------------
		if (nav_bearing > 18000)	nav_bearing -= 36000;
		if (nav_bearing < -18000)	nav_bearing += 36000;
	

	    // Altitude error
	    // --------------
	    if (control_mode > FLY_BY_WIRE_B){
			altitude_error 	= (next_WP.alt - current_loc.alt);
		}
	} 
}

/****************************************************************
Function that will read and store the current altitude when you switch to autopilot mode.
 ****************************************************************/

int get_altitude_above_home(void)
{
	// This is the altitude above the home location
	// The GPS gives us altitude at Sea Level
	// if you slope soar, you should see a negative number sometimes
	// -------------------------------------------------------------
	return current_loc.alt - home.alt;
}

long getDistance(struct Location *loc1, struct Location *loc2)
{
	if(loc1->lat == 0 || loc1->lng == 0) 
		return -1;
	if(loc2->lat == 0 || loc2->lng == 0) 
		return -1;
	float dlat 		= (float)(loc2->lat - loc1->lat);
	float dlong  	= ((float)(loc2->lng - loc1->lng)) * scaleLongUp;
	return sqrt(sq(dlat) + sq(dlong)) * .01113195;
}

long get_alt_distance(struct Location *loc1, struct Location *loc2)
{
	return abs(loc1->alt - loc2->alt);
}

//float getArea(struct Location *loc1, struct Location *loc2)
//{
//	return sq((float)(loc2->lat - loc1->lat)) + (sq((float)(loc2->lng - loc1->lng)) * scaleLongDown);
//}

//long get_bearing2(struct Location *loc1, struct Location *loc2)
//{
//	return 18000 + atan2((float)(loc1->lng - loc2->lng) * scaleLongDown, (float)(loc1->lat - loc2->lat)) * 5729.57795;
//}

long get_bearing(struct Location *loc1, struct Location *loc2)
{
	long off_x = loc2->lng - loc1->lng;
	long off_y = (loc2->lat - loc1->lat) * scaleLongUp;
	long bearing =  9000 + atan2(-off_y, off_x) * 5729.57795;
	if (bearing < 0) bearing += 36000;
	return bearing;
}



