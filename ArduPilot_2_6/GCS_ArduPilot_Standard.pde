//	This is the standard GCS output file for ArduPilot

/*
Message Prefixes
!!!		Position    		Low rate telemetry
+++		Attitude    		High rate telemetry
###		Mode        		Change in control mode
%%%		Waypoint    		Current and previous waypoints
XXX		Alert       		Text alert  - NOTE: Alert message generation is not localized to a function
PPP		IMU Performance		Sent every 20 seconds for performance monitoring

Message Suffix
***    All messages use this suffix
*/

#if GCS_PROTOCOL == 0 || GCS_PROTOCOL == 1 || GCS_PROTOCOL == 6

#if GCS_PROTOCOL != 6
void print_current_waypoints(){
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
#endif

void print_control_mode(void)
{
	switch (control_mode){
		case MANUAL:
			Serial.println("###MANUAL\t0***");
			break;
		case STABILIZE:
			Serial.println("###STABILIZE\t1***");
			break;
		case CIRCLE:
			Serial.println("###CIRCLE\t1***");
			break;
		case FLY_BY_WIRE_A:
			Serial.println("###FLY BY WIRE A\t2***");
			break;
		case FLY_BY_WIRE_B:
			Serial.println("###FLY BY WIRE B\t2***");
			break;
		case AUTO:
			Serial.println("###AUTO\t5***");
			break;
		case RTL:
			Serial.println("###RTL\t6***");
			break;
		case LOITER:
			Serial.println("###LOITER\t7***");
			break;
	}
}


#if GCS_PROTOCOL != 6
void print_position(void)
{
			Serial.print("!!!");
			Serial.print("LAT:");
			Serial.print(current_loc.lat/10,DEC);
			Serial.print(",LON:");
			Serial.print(current_loc.lng/10,DEC); //wp_current_lat
			Serial.print(",SPD:");
			Serial.print(ground_speed/100,DEC);		
			Serial.print(",CRT:");
			Serial.print(climb_rate,DEC);
			Serial.print(",ALT:");
			Serial.print(current_loc.alt/100,DEC);
			Serial.print(",ALH:");
			Serial.print(next_WP.alt/100,DEC);
			Serial.print(",CRS:");
			Serial.print(ground_course/100,DEC);
			Serial.print(",BER:");
			Serial.print(target_bearing/100,DEC);
			Serial.print(",WPN:");
			Serial.print(wp_index,DEC);//Actually is the waypoint.
			Serial.print(",DST:");
			Serial.print(wp_distance,DEC);
			Serial.print(",BTV:");
			Serial.print(battery_voltage,DEC);
			Serial.print(",RSP:");
			Serial.print(servo_roll/100,DEC);
			Serial.print(",TOW:");
			Serial.print(iTOW);
			Serial.println(",***");
			print_telemetry = false;

}


#if GPS_PROTOCOL == 3

void printPerfData(void)
{
    Serial.print("PPP");
    Serial.print("pTm:");
    Serial.print(perf_mon_timer,DEC);
    Serial.print(",mLc:");
    Serial.print(IMU_mainLoop_count,DEC);
    Serial.print(",DtM:");
    Serial.print(G_Dt_max,DEC);
    Serial.print(",gsc:");
    Serial.print(gyro_sat_count,DEC);
    Serial.print(",adc:");
    Serial.print(adc_constraints,DEC);
    Serial.print(",rsc:");
    Serial.print(renorm_sqrt_count,DEC);
    Serial.print(",rbc:");
    Serial.print(renorm_blowup_count,DEC);
    Serial.print(",gpe:");
    Serial.print(gps_payload_error_count,DEC);
    Serial.print(",gce:");
    Serial.print(gps_checksum_error_count,DEC);
    Serial.print(",gpf:");
    Serial.print(gps_pos_fix_count,DEC);
    Serial.print(",gnf:");
    Serial.print(gps_nav_fix_count,DEC);
    Serial.print(",gms:");
    Serial.print(gps_messages_sent,DEC);
    Serial.print(",gmr:");
    Serial.println((gps_messages_received),DEC);
    Serial.print(",ims:");
    Serial.print((IMU_mainLoop_count - gps_messages_sent),DEC);
    Serial.print(",imr:");
    Serial.print((imu_messages_received),DEC);
    Serial.print(",ipe:");
    Serial.print(imu_payload_error_count,DEC);
    Serial.print(",ice:");
    Serial.print(imu_checksum_error_count,DEC);
			Serial.print(",TOW:");
			Serial.print(iTOW);
    Serial.println(",***");
    gps_messages_received = 0;
    imu_messages_received = 0;
    imu_payload_error_count = 0;
    imu_checksum_error_count = 0;
    IMU_mainLoop_count = 0;      //Used as a flag
}
#endif

#endif

#endif


#if GCS_PROTOCOL == 0

void print_attitude(void)
{
	Serial.print("+++");
	Serial.print("ASP:");
	Serial.print(airspeed_current,DEC);
	Serial.print(",THH:");
	Serial.print(servo_throttle,DEC);
	Serial.print (",RLL:");
	Serial.print(roll_sensor/100,DEC);
	Serial.print (",PCH:");
	Serial.print(pitch_sensor/100,DEC);
	#if GPS_PROTOCOL == 3
			Serial.print(",CRS:");
			Serial.print(ground_course/100,DEC);
			Serial.print(",IMU:");
			Serial.print(imu_health,DEC);
	#endif
	Serial.print(",ch3_in:");
	Serial.print(ch3_in);
	Serial.println(",***");

}

#endif

#if GCS_PROTOCOL == 1
void print_attitude(void)
{
//  This section is temporary for debugging.  The real code is below at #else

	Serial.print("+++");
	Serial.print(servo_roll,DEC);
	Serial.print(",");
	Serial.print(nav_roll/100,DEC);
	Serial.print(",");
	Serial.print(roll_sensor/100,DEC);
	Serial.print(",");
	Serial.print(servo_pitch,DEC);
	Serial.print(",");
	Serial.print(nav_pitch/100,DEC);
	Serial.print(",");
	Serial.print(pitch_sensor/100,DEC);
	Serial.print(",");
	Serial.print(target_bearing/100,DEC);
	Serial.print(",");
	Serial.print(nav_bearing/100,DEC);
	Serial.print(",");
	Serial.print(ground_course/100,DEC);
	Serial.print(",");
	Serial.print(airspeed_current,DEC);
	Serial.print(",");
	Serial.print(servo_throttle,DEC);
	Serial.print(",");
#if GPS_PROTOCOL == 3
        Serial.print(imu_health,DEC);
	Serial.print(",");
#endif
	Serial.println(takeoffComplete,DEC);
}
#endif


//***********************************************************************************
//  The following functions are used during startup and are not for telemetry
//***********************************************************************************

void print_radio()
{
	Serial.print("Radio inputs R/A: ");
	Serial.print(ch1_in,DEC);
	Serial.print("\tE: ");
	Serial.print(ch2_in,DEC);
	Serial.print("\tT :");
	Serial.println(ch3_in,DEC);
}

void print_waypoints(byte wp_tot){
	Serial.println("WAYPOINTS IN MEMORY");
	Serial.print("wp_total: ");
	Serial.println(wp_tot, DEC);

	// create a location struct to hold the temp Waypoints for printing
	//Location tmp;
	struct Location tmp = get_loc_with_index(0);
	
	Serial.print("home: \t");
	Serial.print(tmp.lat, DEC);
	Serial.print("\t");
	Serial.print(tmp.lng, DEC);
	Serial.print("\t");
	Serial.println(tmp.alt,DEC);	

	for (int i = 1; i <= wp_tot; i++){
		tmp = get_loc_with_index(i);
		Serial.print("wp #");
		Serial.print(i);
		Serial.print("\t");
		Serial.print(tmp.lat, DEC);
		Serial.print("\t");
		Serial.print(tmp.lng, DEC);
		Serial.print("\t");
		Serial.println(tmp.alt,DEC);
	}
}

void print_launch_params(void)
{
        Serial.println("LAUNCH PARAMETERS");
	Serial.print("wp_index = \t\t");
	Serial.println(wp_index,DEC);
	Serial.print("wp_total = \t\t");
	Serial.println(wp_total,DEC);
	Serial.print("wp_radius = \t\t");
	Serial.println(wp_radius,DEC);
	Serial.print("ch1_trim = \t\t");
	Serial.println(ch1_trim,DEC);
	Serial.print("ch2_trim = \t\t");
	Serial.println(ch2_trim,DEC);
	Serial.print("ch3_trim = \t\t");
	Serial.println(ch3_trim,DEC);	
	Serial.print("ch1_min = \t");
	Serial.println(ch1_min,DEC);
	Serial.print("ch1_max = \t");
	Serial.println(ch1_max,DEC);
	Serial.print("ch2_min = \t");
	Serial.println(ch2_min,DEC);
	Serial.print("ch2_max = \t");
	Serial.println(ch2_max,DEC);
	Serial.print("Home Lat = \t\t");
	Serial.println(home.lat,DEC);
	Serial.print("Home Long = \t\t");
	Serial.println(home.lng,DEC);
	Serial.print("Home altitude = \t");
	Serial.println(home.alt,DEC);
}



