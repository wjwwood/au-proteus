  /***************************************/
 /*ArduPilot 2.6.0 Header file*/
/***************************************/

//HARDWARE CONFIGURATION
//0-1
#define SHIELD_VERSION 1		// Old (red) shield versions is 0, the new (blue) shield version is 1, -1 = no shield
//0-2
#define AIRSPEED_SENSOR 1 		// (boolean) Do you have an airspeed sensor attached? 1= yes, 0 = no.
//0-3
#define GPS_PROTOCOL 0			// 0 = NMEA, 1=SIRF, 2=uBlox, 3 = ArduIMU, 5 = Simulated GPS mode (Debug)
//0-4 Ground Control Station:
#define GCS_PROTOCOL 6			// 0 = LabVIEW/HappyKillmore ground station, 1 = special test, 2 = Ardupilot Binary(not implemented), 5 = Jason's GCS, 6 = XBeeGCS

//0-5 and 0-6 are for use with Thermopile sensors
//0-5
#define ENABLE_Z_SENSOR 0  		// 0 = no Z sensor, 1 = use Z sensor (no Z requires field calibration with each flight)
//0-6
#define XY_SENSOR_LOCATION 0 	//XY Thermopiles Sensor placement
//Mounted right side up: 		0 = cable in front, 1 = cable behind
//Mounted upside down: 			2 = cable in front, 3 = cable behind

//0-6
#define BATTERY_EVENT 0 		// (boolean) 0 = don't read battery, 1 = read battery voltage (only if you have it wired up!)
//0-7
#define INPUT_VOLTAGE 5200.0 	// (Millivolts) voltage your power regulator is feeding your ArduPilot to have an accurate pressure and battery level readings. (you need a multimeter to measure and set this of course)


// Flight Modes
// these Flight modes can be changed either here or directly in events.pde
// options are MANUAL, STABILIZE, FLY_BY_WIRE_A, FLY_BY_WIRE_B, AUTO, RTL, LOITER
//0-8
#define POSITION_1 MANUAL 
//0-9
#define POSITION_2 STABILIZE
//0-10
#define POSITION_3 RTL
// So why isn't AUTO here by default? Well, please try and run Stabilize first, 
// then FLY_BY_WIRE_A to verify you have good gains set up correctly 
// before you try Auto and wreck your plane. I'll sleep better that way...

// FLY_BY_WIRE_A has manual throttle control, FLY_BY_WIRE_B = the throttle stick input controls desired airspeed.
// Fly By Wire B = Fly By Wire A if you have AIRSPEED_SENSOR 0

/***************************************/
// AIRFRAME SETTINGS
//1-1
#define MIXING_MODE 0			//Servo mixing mode 0 = Normal, 1 = Elevons (or v tail)

// NOTE - IF USING ELEVONS, 1-2 AND 1-3 SHOULD BE 1
//1-2
#define REVERSE_ROLL 1			//To reverse roll, PUT -1 to reverse it
//1-3
#define REVERSE_PITCH 1			//To reverse pitch, PUT -1 to reverse it

// JUST FOR ELEVONS:
//1-4
#define REVERSE_ELEVONS 1    	//  Use 1 for regular, -1 if you need to reverse roll direction
//1-5
#define REVERSE_CH1_ELEVON -1 	// To reverse channel 1 elevon servo, PUT -1 to reverse it
//1-6
#define REVERSE_CH2_ELEVON 1 	// To reverse channel 2 elevon servo, PUT -1 to reverse it

//1-7
#define REVERSE_THROTTLE 0		// 0 = Normal mode. 1 = Reverse mode - Try and reverse throttle direction on your radio first, most ESC use low values for low throttle.

//1-8
#define PITCH_TRIM 0			//(Degrees +- 5) allows you to offset bad IR sensor placement
//1-9
#define ROLL_TRIM 0				// (Degrees +- 5) allows you to offset bad IR sensor placement


/***************************************/
// AIRSPEEDS
				// NOTE - Airspeed is stored and used in the program as an integer pressure value
				// Use the formula:  pressure = 0.1254 * speed * speed 
				// where speed is the airspeed in meters per second.
				// For example if you want cruising airspeed to be 20 meters per second use a value of 50
//2-1
#define CRUISE_AIRSPEED 13		// If we have airspeed sensor pitch is used to maintain desired airspeed and throttle is used for climb/descent
//2-2
#define AIRSPEED_FBW_MIN 8		//Minimum airspeed for Fly By Wire mode B, throttle stick at bottom
//2-3
#define AIRSPEED_FBW_MAX 20		//Maximum airspeed for Fly By Wire mode B, throttle stick at top

/***************************************/
//  THROTTLE IN AUTO/RTL MODE
//  In general, you can adjust speed with 2-1 above and ignore 2-4 to 2-8, but to make that more accurate and reliable, you can adjust these as well. The higher your airspeed, the higher your throttle cruise number should be. 

				// NOTE - The range for throttle values is 0 to 125
//2-4
#define THROTTLE_CRUISE 55    	//  Default throttle value - Used for central value.  Failsafe value
          // NOTE - For proper tuning the THROTTLE_CRUISE value should be the correct value to produce CRUISE_AIRSPEED in straight and level flight with your airframe
//2-5
#define THROTTLE_MAX 125        // (0-125) 70 = 56% maximum throttle (lower if your plane is overpowered)
//2-6
#define THROTTLE_MIN 0			// (0-125) (raise it if your plane falls to quickly when decending)
//2-7
#define THROTTLE_IN 1 			// (boolean) Disables throttle input when set to 0
//2-8
#define THROTTLE_OUT 1	 		// 1 = throttle, 0 = no throttle output at all! (good for saving fingers on the bench.)


/***************************************/
// RADIO
//3-1
#define SET_RADIO_LIMITS 0	// 0 = no, 1 = set the limits of the Channels with the radio at launch each time; see manual for more
//3-2
//3-3
#define RADIO_TYPE 0 		// 0 = sequential PWM pulses, 1 = simultaneous PWM pulses
//3-4
#define CH1_MIN 1000 		// (Microseconds) Range of Ailerons/ Rudder
//3-5
#define CH1_MAX 2000 		// (Microseconds)
//3-6
#define CH2_MIN 1000 		// (Microseconds) Range of Elevator
//3-7
#define CH2_MAX 2000 		// (Microseconds)
//3-8
#define CH3_TRIM 1000 		// (Microseconds) Trims are normally set automatically in setup.


/***************************************/
//NAVIGATION: PARAMETERS
//Note: Some Gains are now variables
//4-1
#define HEAD_MAX 4500           	//  The maximum commanded bank angle (left and right) 	degrees*100
//4-2
#define PITCH_MAX 1000				// The maximum commanded pitch up angle 	degrees*100
//4-3
#define PITCH_MIN -1500         	// The maximum commanded pitch down angle 	degrees*100
//4-4
#define XTRACK_GAIN 00 				// amount to compensate for crosstrack (degrees/100 per meter)
//4-5
#define XTRACK_ENTRY_ANGLE 3000		// Max angle used to correct for track following	degrees*100
//4-5
#define LOITER_RADIUS 40 			// radius in meters of a Loiter
//4-6
#define REMEMBER_LAST_WAYPOINT_MODE 0 	//	If set 1 = will remember the last waypoint even if you restart the autopilot. 
										// 	0 = Will start from WP 1 (not 0) every time you switch into AUTO mode. 
//4-7
#define WP_ALTITUDE_RADIUS 10 		// (meters) accuracy needed to find our waypoint in meters   [not implemented]
//4-8
#define ABSOLUTE_WAYPOINTS 1 		// 0 = relative, 1 = absolute [not yet implemented in config utility; must use waypoint writer in test suite to configure]
//4-9
#define AP_STICK_MIXING 1 			// 0 = no mix, 1 = mix.  Mixing lets you "nudge" when in auto modes


/***************************************/
//ATTITUDE: ROLL GAINS [Start with changes of no more than 25% at a time]
//5-1						IMPORTANT!!  Servo Gain values will be 100 times less than equivalent gains for ArduPilot 2.5
#define SERVO_ROLL_P .006				// 	Primary value to tune - overall proportional term determines how much rudder/aileron you use to turn
//5-2
#define SERVO_ROLL_I .0					//	roll PID integrator gain (value should generally be low)
//5-3
#define SERVO_ROLL_D 0.0				//	roll PID derivative gain (for advanced users - should be zero for most airframes)
//5-4
#define SERVO_ROLL_INTEGRATOR_MAX 500	//Maximium integrator value in degrees * 100
//5-5
#define ROLL_SLEW_LIMIT 0				//  Use to limit slew rate of roll servo.  If zero then slew rate is not limited
										//  Value is degree per second limit

/***************************************/
//ATTITUDE: PITCH GAINS [Start with changes of no more than 25% at a time]
//6-1						IMPORTANT!!  Servo Gain values will be 100 times less than equivalent gains for ArduPilot 2.5
#define SERVO_PITCH_P .006				//	Pitch Proportional gain
//6-2
#define SERVO_PITCH_I .0				//	Pitch integrator gain  (value should generally be low)
//6-3
#define SERVO_PITCH_D 0.0				//	Pitch derivative gain  (for advanced users - should be zero for most airframes)
//6-4
#define SERVO_PITCH_INTEGRATOR_MAX  500	//Maximum integrator value in degrees * 100
//6-5
#define PITCH_COMP .10					//Pitch compensation vs. Roll bank angle. 
								// NOTE!!  The implementation of pitch compensation has been changed.
								// The optimal value for your airframe will likely differ between 2.5 and 2.6


/***************************************/
//NAV: ROLL GAINS  [Start with changes of no more than 25% at a time]
//7-1
#define NAV_ROLL_P .6					// 	Primary value to tune - overall proportional term determines how aggressively we bank to change heading
//7-2
#define NAV_ROLL_I .0					//	roll PID integrator gain (value should generally be low)
//7-3
#define NAV_ROLL_D 0.0					//	roll PID derivative gain (for advanced users - should be zero for most airframes)
//7-4
#define NAV_ROLL_INTEGRATOR_MAX 500		//Maximium integrator value in degrees * 100


/***************************************/
//NAV: PITCH GAINS [Start with changes of no more than 25% at a time]
//8-1
#define NAV_PITCH_P .65					// 	Overall proportional term determines how aggressively we change pitch to maintain airspeed
//8-2
#define NAV_PITCH_I .0					//	PID integrator gain (value should generally be low)
//8-3
#define NAV_PITCH_D 0.0					//	PID derivative gain (for advanced users - should be zero for most airframes)
//8-4
#define NAV_PITCH_INTEGRATOR_MAX 500	//Maximium integrator value in degrees * 100


/***************************************/
//ENERGY HEIGHT: THROTTLE OUTPUT GAINS [Start with changes of no more than 25% at a time]
//9-1
#define THROTTLE_P .32					//Proportional
//9-2
#define THROTTLE_I .04					//Integrator
//9-3
#define THROTTLE_D 0.0					//Derivative
//9-4
#define THROTTLE_INTEGRATOR_MAX 20		// (0-125) 70=50% Integrator limit. 
//9-5
#define THROTTLE_SLEW_LIMIT 0			//  Use to limit slew rate of throttle output.  If zero then slew rate is not limited
										//  Value is throttle value (0-125) per second limit


/***************************************/
//FLY BY WIRE AIRSPEED: THROTTLE OUTPUT GAINS 
//10-1
#define THROTTLE_FBW_P .32 			//Proportional
//10-2
#define THROTTLE_FBW_I .04 			//Integrator
//10-3
#define THROTTLE_FBW_D 0.0 			//Derivative
//10-4
#define THROTTLE_FBW_INTEGRATOR_MAX 20 // (0-125) 70=50% Integrator limit. 

  /*****************/
 /*Advanced Stuff*/
/*****************/

//	This section is for auto launch.  This feature is not appropriate for all airframes.
//	If you are using ArduIMU the minimum recommended TAKE_OFF_PITCH is 30 degrees due to linear acceleration effects on the IMU
//  If your airframe cannot climb out at 30 degrees do not use this feature if using ArduIMU
//11-1
#define USE_AUTO_LAUNCH 0		// If set to 1 then in AUTO mode roll will be held to zero and pitch to TAKE_OFF_PITCH until TAKE_OFF_ALT is reached
//11-2
#define TAKE_OFF_ALT 50			// Meters.  Altitude below which take-off controls apply
//11-3
#define TAKE_OFF_PITCH 33		// Pitch value to hold during take-off in degrees

//	This section is for setting up auto landings
//	You must have your airframe tuned well and plan your flight carefully to successfully execute auto landing
//11-4
#define SLOW_WAYPOINT 998			// When this becomes the current waypoint we will decrease cruise_airspeed to AIRSPEED_SLOW. Replace 999 with the beginning of your landing pattern 
//11-5
#define AIRSPEED_SLOW 10
//11-6
#define THROTTLE_SLOW 30    	//  This should be the throttle value that produces AIRSPEED_SLOW in straight and level flight
//11-7
#define THROTTLE_CUT_WAYPOINT 999	// When this becomes the current waypoint we will cut the throttle; set it so it is well beyond the touchdown zone so that it is not reached, else you will enter RTL mode or loop waypoints
				    // Remember that this is engaged with it becomes the next waypoint to go to, not when it is reached. If it is reached, the plane will climb again and RTL.
  /*****************/
 /*Debugging Stuff for Sim Mode*/
/*****************/
//12-1
#define TURNRATE 85 // (degrees) how fast we turn per second in degrees at full bank
//12-2
#define CLIMBRATE_UP 1000 // (meters * 100) how fast we climb in simulator at 90° 
//12-3
#define CLIMBRATE_DOWN 3000 // (meters * 100) how fast we climb in simulator at 90° 


/***************/
/* Stuff added by us */
/***************/
#define XBEE_BAUD_RATE  115200
#define XBEE_RTS_PIN  14
#define GCS_MAX_PACKET_SIZE  16
#define GPS_BAUD_RATE  FIFTY_SEVEN_K_BAUD

/*
typedef struct {
  struct Location current_loc;    
	struct Location next_WP;				
	long wp_distance;						
	float ground_speed;					
	long roll;											
	long pitch;
} packet_t;
*/

