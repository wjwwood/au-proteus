/*
	This event will be called when the failsafe changes
	boolean failsafe reflects the current state
*/
void failsafe_event()
{
	if (failsafe == true){
		// This is how to handle a failsafe.
		// 
		switch(control_mode)
		{
			case MANUAL: // First position
			set_mode(STABILIZE);
			break;
	
			case STABILIZE: // middle position
			if (GPS_fix != FAILED_GPS){
				set_mode(RTL);
			}else{
				set_mode(CIRCLE);
			}
			break;
	
			case FLY_BY_WIRE_A: // middle position
			if (GPS_fix != FAILED_GPS){
				set_mode(RTL);
			}else{
				set_mode(CIRCLE);
			}
			break;
			
			case FLY_BY_WIRE_B: // middle position
			if (GPS_fix != FAILED_GPS){
				set_mode(RTL);
			}else{
				set_mode(CIRCLE);
			}
			break;

			case CIRCLE: // middle position
			break;

			case AUTO: // middle position
			break;

			case RTL: // middle position
			break;

			case LOITER: // middle position
			break;
			
		}
	}else{
		reset_I();
	}
}

/*
	This event will be called when the switch changes
	It is up to the user how to react to a swicth change event
	options are: MANUAL, STABILIZE, FLY_BY_WIRE_A, FLY_BY_WIRE_B, AUTO, RTL, LOITER 
	see: defines.h
	
	The three switch postions can be handled by most radios.
	Adjust your seetings to make sure all three positions work.
	If you don't have a 3 postion switch, try a two position one 
	and note which case below activates in each position.
*/
void switch_event(byte switchPosition)
{
	switch(switchPosition)
	{
		case 1: // First position
		set_mode(POSITION_1);
		break;

		case 2: // middle position
		/*
			if (GPS_fix != FAILED_GPS){
				set_mode(RTL);
			}else{
				set_mode(CIRCLE);
			}
		*/
		set_mode(POSITION_2);
		break;

		case 3: // last position
		set_mode(POSITION_3);
		break;
	}
}

void waypoint_event(byte event)
{
	switch(event)
	{
		case EVENT_WILL_REACH_WAYPOINT:
			// called just before wp_index is incemented
			Serial.print("XXX \t Reached WP:");
			Serial.print(wp_index,DEC);
			Serial.println(" \t ***");
			break;
			
		case EVENT_SET_NEW_WAYPOINT_INDEX:
			// called just after wp_index is incemented
			Serial.print("XXX \t Now going to WP:");
			Serial.print(wp_index,DEC);
			Serial.println(" \t ***");
			break;

		case EVENT_LOADED_WAYPOINT:
			//Serial.print("Loaded WP index:");
			//Serial.println(wp_index,DEC);
			print_current_waypoints();
			reset_I();
			
			break;
			
		// called when the pattern to be flown is automatically restarted
		case EVENT_LOOP: 
			Serial.println("XXX \t Looped WP Index \t ***");
			//print_current_waypoints();
			break;			
			
	}
}

void gps_event(void)
{


}

// called after every single control loop
void mainLoop_event(void)
{
/*
	if (control_mode == LOITER){
		if (wp_index == 2 && elapsedTime > 120000 ){ // 2 minutes
			elapsedTime = 0;
			// our waypoints index is not altered during LOITER
			// All we need to do is reload the waypoint
			load_waypoint();
			// and return to Autopilot mode!
			set_mode(AUTO);
		}
	}
*/
}

void low_battery_event(void)
{
	Serial.println("XXX \t Low Battery \t ***");
	set_mode(RTL);
}



		
