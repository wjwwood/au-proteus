#if GCS_PROTOCOL == 5
// this is my personal GCS - Jason

void pipe()
{
	Serial.print("|");
}

void print_current_waypoints()
{
	Serial.print("PWP:");
	Serial.print("\t");
	Serial.print(prev_WP.lat,DEC);
	Serial.print(",\t");
	Serial.print(prev_WP.lng,DEC);
	Serial.print(",\t");
	Serial.println(prev_WP.alt,DEC);

	Serial.print("CUR:");
	Serial.print("\t");
	Serial.print(current_loc.lat,DEC);					
	Serial.print(",\t");
	Serial.print(current_loc.lng,DEC);					
	Serial.print(",\t");
	Serial.println(current_loc.alt,DEC);					
	
	Serial.print("NWP:");
	Serial.print(wp_index,DEC);
	Serial.print(",\t");
	Serial.print(next_WP.lat,DEC);
	Serial.print(",\t");
	Serial.print(next_WP.lng,DEC);
	Serial.print(",\t");
	Serial.println(next_WP.alt,DEC);
}

void print_position(void)
{
	Serial.print("!!");
	Serial.print(current_loc.lat,DEC);					// 0
	pipe();
	Serial.print(current_loc.lng,DEC);					// 1
	pipe();
	Serial.print(current_loc.alt,DEC);					// 2
	pipe();
	Serial.print(ground_speed,DEC);						// 3
	pipe();
	Serial.print(airspeed_current,DEC);					// 4
	pipe();
	Serial.print(get_altitude_above_home(),DEC);		// 5
	pipe();
	Serial.print(climb_rate,DEC);						// 6
	pipe();
	Serial.print(wp_distance,DEC);						// 7
	pipe();
	Serial.print(throttle_cruise,DEC);					// 8
	pipe();
	Serial.println(altitude_error,DEC);					// 9
}

void print_attitude(void)
{
	Serial.print("++");
	Serial.print(ch1_out,DEC);						// 0
	pipe();
	Serial.print(ch2_out,DEC);						// 1
	pipe();
	Serial.print(ch3_out,DEC);						// 2
	pipe();
	Serial.print(roll_sensor,DEC);					// 3
	pipe();
	Serial.print(pitch_sensor,DEC);					// 4
	pipe();
	Serial.print(ir_max,DEC);						// 5
	pipe();
	Serial.print(ground_course,DEC);				// 6
	pipe();
	Serial.print(target_bearing,DEC);				// 7
	pipe();
	Serial.println(nav_roll,DEC);					// 8
	
}

// required by Groundstation to plot lateral tracking course 
void print_new_wp_info()
{
	Serial.print("??");
	Serial.print(wp_index,DEC);			//0
	pipe();
	Serial.print(prev_WP.lat,DEC);		//1
	pipe();
	Serial.print(prev_WP.lng,DEC);		//2
	pipe();
	Serial.print(prev_WP.alt,DEC);		//3
	pipe();
	Serial.print(next_WP.lat,DEC);		//4
	pipe();
	Serial.print(next_WP.lng,DEC);		//5
	pipe();
	Serial.print(next_WP.alt,DEC);		//6
	pipe();
	Serial.print(wp_totalDistance,DEC);	//7
	pipe();
	Serial.print(ch1_trim,DEC);			//8
	pipe();
	Serial.println(ch2_trim,DEC);		//9
}

void print_control_mode(void)
{
	switch (control_mode){
		case MANUAL:
			Serial.println("##MANUAL");
			break;
		case STABILIZE:
			Serial.println("##STABILIZE");
			break;
		case CIRCLE:
			Serial.println("##CIRCLE");
			break;
		case FLY_BY_WIRE_A:
			Serial.println("##FBW A");
			break;
		case FLY_BY_WIRE_B:
			Serial.println("##FBW B");
			break;
		case AUTO:
			Serial.println("##AUTO");
			break;
		case RTL:
			Serial.println("##RTL");
			break;
		case LOITER:
			Serial.println("##LOITER");
			break;
	}
}

#endif