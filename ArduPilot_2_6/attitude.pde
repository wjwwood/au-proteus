/****************************************************************
	Function that controls rudder/aileron, elevator and throttle to produce desired attitude and airspeed.
 ****************************************************************/


void stabilize()
{

	float ch1_inf = 1.0;
	float ch2_inf = 1.0;
	
	if(GPS_fix == FAILED_GPS && control_mode > FLY_BY_WIRE_B){		
		nav_roll = HEAD_MAX / 3;   		// If we have lost gps and have no ability to navigate we will circle at a gentle bank angle
	}									// This is another form of failsafe, different from losing radio signal.

	if ((AP_STICK_MIXING == 1 && control_mode > FLY_BY_WIRE_B) || control_mode < FLY_BY_WIRE_A) {
		ch1_inf = (float)ch1_in - (float)ch1_trim;
		ch1_inf = abs(ch1_inf);
		ch1_inf = min(ch1_inf, 100.0);
		ch1_inf = ((100.0 - ch1_inf) /100.0);
		
		ch2_inf = (float)ch2_in - (float)ch2_trim;
		ch2_inf = abs(ch2_inf);									
		ch2_inf = min(ch2_inf, 100.0);							
		ch2_inf = ((100.0 - ch2_inf) /100.0);
	}
	
	// ------------------------------------------------
	if (wp_index >= SLOW_WAYPOINT) {
		cruise_airspeed = AIRSPEED_SLOW;
		throttle_cruise = THROTTLE_SLOW;
	}
	
	if (takeoffComplete == 1) {
		if (control_mode > FLY_BY_WIRE_B && AIRSPEED_SENSOR) {
			airspeed_error = cruise_airspeed - airspeed_current;
			// Remember airspeed values are really a pressure value, not m/s            
			// NOTE - Airspeed is stored and used in the program as an integer pressure value
			// Use the formula:  pressure = 0.1254 * speed * speed 
			// where speed is the airspeed in meters per second.
			// For example if you want airspeed_min to be 10 meters per second use a value of 13
			
			// We scale airspeed_error up by 100 in the PID function call to be same order of magnitude as altitude error (centimeters)
			// in the no airspeed sensor case for consistency
			nav_pitch = - PID((airspeed_error*100), dTnav, CASE_NAV_PITCH);
			
			// pitch down is negative - Pitch up to climb (below), Pitch up to go faster (above)

	  } else if ((control_mode > FLY_BY_WIRE_B) && !AIRSPEED_SENSOR) {
			nav_pitch = PID(altitude_error, dTnav, CASE_NAV_PITCH);
	  }
	  nav_pitch = constrain(nav_pitch,PITCH_MIN,PITCH_MAX);
	}

	// Calculate dersired servo output for the roll 
	// ---------------------------------------------
	servo_roll	= ch1_inf * PID((nav_roll - roll_sensor),   deltaMiliSeconds, CASE_SERVO_ROLL);
	servo_pitch = ch2_inf * PID((nav_pitch + abs(roll_sensor * PITCH_COMP) - pitch_sensor), deltaMiliSeconds, CASE_SERVO_PITCH);  
	//Serial.print("servo_roll ");
	//Serial.println(servo_roll,DEC);
	

	// Mix in stick inputs in STABILIZE mode or if AP_STICK_MIXING = 1
	// ---------------------------------
	servo_roll  +=  REVERSE_ROLL * (1.0f - ch1_inf) * (ch1_in - ch1_trim) * 45.0f / 400;
	servo_pitch	+=  REVERSE_PITCH * (1.0f - ch2_inf) * (ch2_in - ch2_trim) * 45.0f / 400;


	// Call slew rate limiter if used
	#if(ROLL_SLEW_LIMIT != 0)
		servo_roll      = roll_slew_limit(servo_roll);
	#endif

	// throttle control with airspeed compensation   
	// -------------------------------------------
	if ((control_mode > FLY_BY_WIRE_B) && AIRSPEED_SENSOR) {
		if (wp_index >= THROTTLE_CUT_WAYPOINT) {
		
			servo_throttle = 0;
		} else {
			long total_energy_error = (long)(((float)airspeed_error / 0.1254f) + ((float)altitude_error * 0.0981f));   
			//  This is our total energy error in meters squared per second squared
			//  v**2 + alt*g   (kinetic + potential)
			//  Need to correct the kinetic with Taylor expansion?
			
			total_energy_error = max(total_energy_error, 0);
			servo_throttle = throttle_cruise + PID(total_energy_error,dTnav,CASE_TE_THROTTLE);
			servo_throttle = constrain(servo_throttle, THROTTLE_MIN, THROTTLE_MAX);
		}
		
	} else if ((control_mode > FLY_BY_WIRE_B) && !AIRSPEED_SENSOR) {
		// no airspeed sensor
		// ------------------
		if (nav_pitch >= 0) {
			servo_throttle = THROTTLE_CRUISE + (THROTTLE_MAX - THROTTLE_CRUISE) * nav_pitch / PITCH_MAX;
		} else {
			servo_throttle = THROTTLE_CRUISE - (THROTTLE_CRUISE - THROTTLE_MIN) * nav_pitch / PITCH_MIN;
		}
		
	} else if ((control_mode == FLY_BY_WIRE_B) && AIRSPEED_SENSOR) {
		airspeed_error = (servo_throttle/125 * ( AIRSPEED_FBW_MAX - AIRSPEED_FBW_MIN)) + AIRSPEED_FBW_MIN - airspeed_current;
		servo_throttle = PID(airspeed_error,dTnav,CASE_FBW_THROTTLE);       
	} 

	#if(THROTTLE_SLEW_LIMIT != 0)
		servo_throttle = throttle_slew_limit(servo_throttle);
	#endif
	
	if(GPS_fix == GPS_IMU_ERROR && control_mode > MANUAL){	        //  We have lost the IMU - Big trouble
		servo_roll = 0;   											//  If we have lost imu we will probably crash.  
		servo_pitch = 0;											//  Neutralize controls, throttle off
		servo_throttle = 0;
	}   

	// write out the servo PWM values
	// ------------------------------
	set_degrees_mix();
	update_throttle();
}


void calc_nav_roll() 
{

	#define TARGET_AIRSPEED sqrt(CRUISE_AIRSPEED / 0.1254)
	
	#if (GPS_PROTOCOL != 3 && AIRSPEED_SENSOR )			
		//If we don't have IMU we will estimate ground course between fixes
		float calc_airspeed = sqrt(airspeed_current / 0.1254);

		// w = g tan(b) / v     (degrees*100)/second
		float calc_turn_rate = 100. * 57.29 * (9.8 * tan(radians(roll_sensor/100))) / calc_airspeed;

		//  Need formula for turn rate based on airspeed and bank angle
		ground_course_est += constrain(calc_turn_rate * dTnav / 1000.f, -1000, 1000) ;	

		if (ground_course_est > 18000)	ground_course_est -= 36000;
		if (ground_course_est < -18000)	ground_course_est += 36000;
		
		bearing_error = nav_bearing - ground_course_est;	
	#else
		bearing_error = nav_bearing - ground_course;	
	#endif
	
	if (bearing_error > 18000)	bearing_error -= 36000;
	if (bearing_error < -18000)	bearing_error += 36000;
	
	// Adjust gain based on ground speed - We need lower nav gain going in to a headwind, etc.
	// This does not make provisions for wind speed in excess of airframe speed
	nav_gain_scaler = (ground_speed/100) / TARGET_AIRSPEED;
	nav_gain_scaler = constrain(nav_gain_scaler,0.6,1.3);
	
	kp[CASE_NAV_ROLL] = NAV_ROLL_P * nav_gain_scaler;	
	ki[CASE_NAV_ROLL] = NAV_ROLL_I * nav_gain_scaler;	
	kd[CASE_NAV_ROLL] = NAV_ROLL_D * nav_gain_scaler;
	
	// negative error = left turn
	// positive error = right turn
	// Calculate the required roll of the plane
	// ----------------------------------------
	nav_roll = PID(bearing_error, dTnav,CASE_NAV_ROLL);  //returns desired bank angle in degrees*100
	nav_roll = constrain(nav_roll,-HEAD_MAX,HEAD_MAX);

}

float roll_slew_limit(float servo)
{
		static float last;
		float temp = constrain(servo, last-ROLL_SLEW_LIMIT * deltaMiliSeconds/1000.f, last + ROLL_SLEW_LIMIT * deltaMiliSeconds/1000.f);
		last = servo;
		return temp;
}

float throttle_slew_limit(float throttle)
{
		static float last;
		float temp = constrain(throttle, last-THROTTLE_SLEW_LIMIT * deltaMiliSeconds/1000.f, last + THROTTLE_SLEW_LIMIT * deltaMiliSeconds/1000.f);
		last = throttle;
		return temp;
}
	


