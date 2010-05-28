#if GCS_PROTOCOL == 5
// this is my personal GCS - Jason

void print_current_waypoint()
{
		Serial.print("%%%");
		Serial.print("PWP:");
		Serial.print("\t\t");
		Serial.print(prev_WP.lat,DEC);
		Serial.print(",\t");
		Serial.print(prev_WP.lng,DEC);
		Serial.print(",\t");
		Serial.print(prev_WP.alt,DEC);
		
		Serial.print(",\nNWP:");
		Serial.print(wp_index,DEC);
		Serial.print(",\t");
		Serial.print(next_WP.lat,DEC);
		Serial.print(",\t");
		Serial.print(next_WP.lng,DEC);
		Serial.print(",\t");
		Serial.print(next_WP.alt,DEC);
		Serial.println(",***");
}

void print_position(void)
{
	//!!377659260|-1224329073|5543|79|-56|5543|0|7982
	Serial.print("!!");
	Serial.print(current_loc.lat,DEC);					// 0
	Serial.print("|");
	Serial.print(current_loc.lng,DEC);					// 1
	Serial.print("|");
	Serial.print(current_loc.alt,DEC);					// 2
	Serial.print("|");
	Serial.print(ground_speed,DEC);					// 3
	Serial.print("|");	
	Serial.print(airspeed_current,DEC);			// 4
	Serial.print("|");
	Serial.print(get_altitude_above_home(),DEC);	// 5
	Serial.print("|");
	Serial.print(climb_rate,DEC);					// 6
	Serial.print("|");
	Serial.print(wp_distance,DEC);					// 7
	Serial.print("|");
	Serial.print(nav_airspeed,DEC);					// 8
	Serial.print("|");
	Serial.print(throttle_cruise,DEC);					// 9
	Serial.print("|");
	Serial.println(altitude_error,DEC);					// 10
}
void print_attitude(void)
{
	Serial.print("++");
	Serial.print(ch1_out,DEC);
	Serial.print("|");
	Serial.print(ch2_out,DEC);
	Serial.print("|");
	Serial.print(ch3_out,DEC);
	Serial.print("|");
	Serial.print(roll_sensor,DEC);
	Serial.print("|");
	Serial.print(pitch_sensor,DEC);
	Serial.print("|");
	Serial.print(ir_max,DEC);				// 9
	Serial.print("|");
	Serial.print(ground_course,DEC);				// 9
	Serial.print("|");
	Serial.print(target_bearing,DEC);				// 10
	Serial.print("|");
	Serial.print(nav_roll,DEC);				// 10
	Serial.print("|");
	Serial.print(derivative_roll,DEC);				// 10
	Serial.println("|");
	
}

// required by Groundstation to plot lateral tracking course 
void print_new_wp_info()
{
	Serial.print("??");
	Serial.print(wp_index,DEC);			//0
	Serial.print("|");
	Serial.print(prev_WP.lat,DEC);		//1
	Serial.print("|");
	Serial.print(prev_WP.lng,DEC);		//2
	Serial.print("|");
	Serial.print(prev_WP.alt,DEC);		//3
	Serial.print("|");
	Serial.print(next_WP.lat,DEC);		//4
	Serial.print("|");
	Serial.print(next_WP.lng,DEC);		//5
	Serial.print("|");
	Serial.print(next_WP.alt,DEC);		//6
	Serial.print("|");
	Serial.print(wp_totalDistance,DEC);	//7
	Serial.print("|");
	Serial.print(ch1_trim,DEC);			//8
	Serial.print("|");
	Serial.println(ch2_trim,DEC);		//9
}

#endif