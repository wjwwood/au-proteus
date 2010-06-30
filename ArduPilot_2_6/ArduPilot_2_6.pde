#include <avr/io.h>
#include <avr/eeprom.h>
#include <math.h>
#include "NewSoftSerial.h"
#include "defines.h"

//To use the header file in your library, use brackets:
//#include <ap_2_6_header.h>

//To use the header file in your local folder, use quotes:
#include "AP_2_6_header.h"
//#include "easystar_25.h"
//#include "SkyFun_2.h"

 /*
ArduPilot By Jordi Munoz
ArduPilot Version 2.5 By Jason Short
ArduPilot Version 2.6 By Doug Weibel
ArduPilot Version 2.6.1 By Doug Weibel & Jason Short
Developed by
	-Chris Anderson
	-Jordi Munoz
	-Jason Short
	-Doug Weibel
	-HappyKillMore
	-Jose Julio	
	-Bill Premerlani
	-James Cohen.
	-JB from rotorFX.
	-Automatik.
	-Fefenin
	-Peter Meister
	-Remzibi
	-Chester Hamilton
	-Varun Sampath

*/


// set by the control switch read by the ATTiny
// --------------------------------------------
byte control_mode			= MANUAL;
boolean invalid_location 	= true;		// used to indicate we can't navigate witout good GPS data - the equations will choke

// Radio values
// ------------
int ch1_trim = 1500;			// set by init sequence
int ch2_trim = 1500;			// set by init sequence
int elevon1_trim = 1500;
int elevon2_trim = 1500;
int ch3_trim = CH3_TRIM;	// set by init sequence unless THROTTLE_IN == 0
int ch3_timer_trim = 0; 	// set by init sequence
int ch3_fs;					// set your failsafe throttle >50\u00b5s below ch3 trim

int ch1_temp = 1500;		// Used for elevon mixing
int ch2_temp = 1500;
int ch1_in = 1500;			// store aileron/rudder position from radio
int ch2_in = 1500;			// store elevator position from radio
int ch3_in = CH3_TRIM;		// store thottle position from radio
// for elevons ch1_in and ch2_in are equivalent aileron and elevator, not left and right elevon

int ch1_out = 1500;				// actual µs values to servos
int ch2_out = 1500;				// actual µs values to servos
int ch3_out = CH3_TRIM;			// actual µs values to servos

// servo limits - set during initialization
// ----------------------------------------
int ch1_min		= CH1_MIN;		// lowest 	~ 1000
int ch1_max		= CH1_MAX; 		// highest 	~ 2000
int ch2_min		= CH2_MIN;		//
int ch2_max		= CH2_MAX;		//
int ch3_min		= 0;			//

// If your radio receiver has failsafes set the Throttle to go below the trim value.
// We look for this condition to signal failsafe
// ----------------------------------------------------------------
boolean failsafe			= false;	// did our throttle dip below the failsafe value?
byte 	config_tool_options	= 0;		// 

//	PID Control variables
//-----------------------
float kp[]={SERVO_ROLL_P,SERVO_PITCH_P,NAV_ROLL_P,NAV_PITCH_P,THROTTLE_P,THROTTLE_FBW_P}		
		,ki[]={SERVO_ROLL_I,SERVO_PITCH_I,NAV_ROLL_I,NAV_PITCH_I,THROTTLE_I,THROTTLE_FBW_I}
		,kd[]={SERVO_ROLL_D,SERVO_PITCH_D,NAV_ROLL_D,NAV_PITCH_D,THROTTLE_D,THROTTLE_FBW_D}; 		//PID gains

const float Integrator_max[]={SERVO_ROLL_INTEGRATOR_MAX,SERVO_PITCH_INTEGRATOR_MAX,NAV_ROLL_INTEGRATOR_MAX,NAV_PITCH_INTEGRATOR_MAX,THROTTLE_INTEGRATOR_MAX,THROTTLE_FBW_INTEGRATOR_MAX};

float	 Integrator[]={0,0,0,0,0,0};	//PID Integrators
float	 Last_error[]={0,0,0,0,0,0};	//PID last error for derivative


// attitude control output
// -----------------------
float 	servo_roll			= 0;	 		// degrees to servos
float 	servo_pitch			= 0;	 		// degrees to servos
int 	servo_throttle		= 0;			// 0-125 value

// GPS variables
// -------------
int 	GPS_flag			= -1;			// have we achieved first lock and set Home?
boolean GPS_light			= false;		// status of the GPS light
byte 	GPS_fix				= BAD_GPS;		// This variable store the status of the GPS
float 	ground_speed 		= 0;			// centimeters/second
float 	climb_rate 			= 0;			// meters/second
byte 	GPS_update			= GPS_NONE;		// do we have GPS data to work with?
const float t7				= 10000000.0;	// used to scale GPS values for EEPROM storage
boolean print_telemetry		= false;
long 	iTOW 				= 0; //GPS Millisecond Time of Week

// navigation 
// ----------
long 	ground_course 		= 0;			// degrees * 100 dir of plane
long	ground_course_est	= 0;			// used for control calculations, degrees * 100
long 	target_bearing		= 0;			// degrees * 100 location of the plane to the target
long	nav_bearing			= 0;			// degrees * 100 current desired bearing to navigate
long 	bearing_error		= 0; 			// degrees * 100
long 	crosstrack_bearing	= 0;			// degrees * 100 location of the plane to the target
float	crosstrack_error	= 0;			// meters we are off trackline
int 	altitude_error		= 0;			// meters * 100 we are off in altitude
int 	max_altitude		= 0;			// meters - read by config tool!
byte 	max_speed			= 0;			// m/s
byte 	wp_radius			= 15;			// meters - set by config tool!
boolean wp_mode				= ABS_WP;		// ABS_WP or REL_WP
float 	nav_gain_scaler		= 1;			// Gain scaling for headwind/tailwind
#if USE_AUTO_LAUNCH == 0
	boolean takeoffComplete		= 1;			// Flag for using take-off controls
#else
	boolean takeoffComplete		= 0;
#endif

// these are the values returned from navigation control functions
// ----------------------------------------------------
long 	nav_roll			= 0;					// target roll angle in degrees * 100
long 	nav_pitch			= 0;					// target pitch angle in degrees * 100
long 	airspeed_error		= 0;
int 	throttle_cruise		= THROTTLE_CRUISE;		// target airspeed sensor value - throttle_cruise = airspeed at cruising
long	cruise_airspeed		= CRUISE_AIRSPEED;		// target airspeed sensor value

// used to consruct the GPS data from Bytes to ints and longs
// ----------------------------------------------------------
union long_union {
	int32_t dword;
	uint8_t	byte[4];
} longUnion;

union int_union {
	int16_t word;
	uint8_t	byte[2];
} intUnion;



// System Timers
// --------------
unsigned long fast_loopTimer		= 0;		// Time in miliseconds of main control loop
unsigned long slow_loopTimer		= 0;		// Time in miliseconds of navigation control loop
byte medium_loopCounter				= 0;		// Counters for branching from main control loop to slower loops
byte slow_loopCounter				= 0;		// 
unsigned long deltaMiliSeconds 		= 0;		// Delta Time in miliseconds
unsigned long dTnav					= 0;		// Delta Time in milliseconds for navigation computations
unsigned long elapsedTime 			= 0;		// in miliseconds
int IMU_mainLoop_count = 0;


// Waypoints
// ---------
long 	wp_distance			= 0;	// meters - distance between plane and next waypoint
long 	wp_totalDistance	= 0;	// meters - distance between old and next waypoint
byte 	wp_total			= 0;	// # of waypoints
byte 	wp_index			= 0;	// Current WP index, -1 is RTL
float 	scaleLongUp			= 0;	// used to reverse longtitude scaling

// XXX: Added for arbitrary waypoint loading
boolean fakeWP = false;

// 3D Location vectors
// -------------------
struct Location {
	long lat;
	long lng;
	long alt;
};

struct Location home 				= {0,0,0};		// home location
struct Location prev_WP 			= {0,0,0};		// last waypoint
struct Location current_loc			= {0,0,0};		// current location
//struct Location est_loc 			= {0,0,0};		// for estimation
struct Location next_WP 			= {0,0,0};		// next waypoint

// Sensors 
// --------
long analog0				= 511;		// Thermopiles - Pitch
long analog1				= 511;		// Thermopiles - Roll
long analog2				= 511;		// Thermopiles - Z
float analog3				= 511;		// Airspeed Sensor - is a float to better handle filtering
float analog5				= 511;		// Battery Voltage
float battery_voltage 		= 0;
int ir_max					= 300;		// used to scale Thermopile output to 511
int ir_max_save				= 300;		// used to scale Thermopile output to 511
long roll_sensor			= 0;		// how much we're turning in degrees * 100
long pitch_sensor			= 0;		// our angle of attack in degrees * 100
int airspeed_offset			= 0;		// read the analog airspeed sensors to get this
long airspeed_current		= 0;		// airspeed as a pressure value

// IMU specific
// ------------
#if GPS_PROTOCOL == 3
long perf_mon_timer = 0;
byte imu_health = 0;
int G_Dt_max = 0.0;						//Max main loop cycle time in milliseconds
byte gyro_sat_count = 0;
byte adc_constraints = 0;
byte renorm_sqrt_count = 0;
byte renorm_blowup_count = 0;
byte gps_payload_error_count = 0;
byte gps_checksum_error_count = 0;
byte gps_pos_fix_count = 0;
byte gps_nav_fix_count = 0;
byte gps_messages_sent = 0;
byte gps_messages_received = 0;
int imu_messages_received = 0;
byte imu_payload_error_count = 0;
byte imu_checksum_error_count = 0;
#endif

// Debugging
// ---------
long est_turn_rate			= 0;
long actual_turn_rate		= 0;

#ifdef XBEE_READ
// Packet
// ------
struct GCS_packet_t {
  struct Location next_WP;  // 12 bytes
  long checksum;	          // 4 bytes
};

struct GCS_packet_t pkt;
int val;
NewSoftSerial xbeeSerial (XBEE_TX_PIN, XBEE_TX_PIN);
#endif

// Basic Initialization
//---------------------
void setup() {
	delay(3000); // delay of three seconds so we don't brick the Xbee module
	
	#if GPS_PROTOCOL == -1
		Serial.begin(THIRTY_EIGHT_K_BAUD);
	#endif
	#if GPS_PROTOCOL == 0
		Serial.begin(FIFTY_SEVEN_K_BAUD);
	#endif
	#if GPS_PROTOCOL == 1
		Serial.begin(FIFTY_SEVEN_K_BAUD);
	#endif
	#if GPS_PROTOCOL == 2
		Serial.begin(THIRTY_EIGHT_K_BAUD);
	#endif
	#if GPS_PROTOCOL == 3
		Serial.begin(THIRTY_EIGHT_K_BAUD);  
	#endif
	#if GPS_PROTOCOL == 5
		Serial.begin(THIRTY_EIGHT_K_BAUD);
	#endif
	init_ardupilot();

	// Initialize XBee TX setup
	// ------------------------
#ifdef XBEE_READ
	init_xbee();
#endif

}


void loop()
{
	// We want this to execute at 50Hz if possible
	// -------------------------------------------
	if (DIYmillis()-fast_loopTimer > 19) {
		deltaMiliSeconds 	= DIYmillis() - fast_loopTimer;
		fast_loopTimer		= DIYmillis();

		// Execute the fast loop 
		// ---------------------
		control_loop();

		// send a notification to the events.pde file
		// ------------------------------------------
		mainLoop_event();
	}
}

void control_loop()
{
	// Read 3-position switch on radio (usually Ch5)
	// -------------------------------------------------
	read_control_switch();

	// Filters radio input - adjust filters in the radio.pde file
	// ----------------------------------------------------------
	read_radio();
	
	// check for throtle failsafe condition
	// ------------------------------------
	#if THROTTLE_FAILSAFE == 1
	throttle_failsafe();
	#endif
		
	// read in the plane's attitude
	// ----------------------------
	#if GPS_PROTOCOL == 3
		// We are using the IMU
		// ---------------------
		decode_gps();
		
	#elif GPS_PROTOCOL == 5
		// this is the sim
		// ---------------
		read_sim_sensors();
		
	#else
		// 	We are using FMA sensors
		// ------------------------
		read_XY_sensors();
	
		// if we are using a Z sensor, read that in too
		// --------------------------------------------
		#if ENABLE_Z_SENSOR == 1
		read_z_sensor();
		#endif
	#endif
	
	// Read Airspeed
	// -------------
	#if AIRSPEED_SENSOR == 1 && GPS_PROTOCOL != 5  // 5 - sim
	read_airspeed();
	#endif
	
	// custom code/exceptions for flight modes
	// ---------------------------------------
	update_current_flight_mode();
	
	// Disable roll navigation during auto takeoff
	// -------------------------------------------
	check_for_takeoffComplete();	
	
	// apply desired roll and pitch to the plane
	// -----------------------------------------
	stabilize();
	
	// This is the start of the medium (10 Hz) loop pieces
	// -----------------------------------------
	switch (medium_loopCounter) {
		case 0:
			medium_loopCounter++;
			print_attitude();
			break;

		case 1:
			medium_loopCounter++;
			// only output location if we have GPS lock
			// ----------------------------------------
			update_telemetry();
			break;

		case 2:
			medium_loopCounter++;
			
			// update navigation timer
			// -----------------------			
			dTnav = DIYmillis() - slow_loopTimer;
			slow_loopTimer = DIYmillis();
			
			// find out how much to roll the plane based on our bearing error
			// --------------------------------------------------------------
			calc_nav_roll();
			
			break;

		case 3:
			medium_loopCounter++;
			break;
			// Reserved

		case 4:
			medium_loopCounter=0;
			//	This is the start of the slow (3 1/3 Hz) loop pieces
			switch (slow_loopCounter) {
				case 0:
					slow_loopCounter++;
					// Read main battery voltage if hooked up - does not read the 5v from radio
					// ------------------------------------------------------------------------
					#if BATTERY_EVENT == 1
					read_battery();
					#endif

#ifdef XBEE_READ
					// XBee reading time
					val = xbee_read(&pkt);
					if (val > 0)
						load_waypoint(&(pkt.next_WP));
#ifdef XBEE_DEBUG
					else
						Serial.println("Checksum fail!");
#endif
#endif
					break;
					
				case 1:
					slow_loopCounter++;
					// calculate the plane's desired bearing
					// -------------------------------------
					navigate();
					break;
					
				case 2:
					slow_loopCounter = 0;
					// Reserved
					break;
			}
			break;	
	}

	#if GPS_PROTOCOL == 3
		if (IMU_mainLoop_count != 0){
			printPerfData();
		}
	#endif
}


void update_telemetry(void)
{
	#if GPS_PROTOCOL != 3
	decode_gps();
	#endif

	// Blink GPS LED if we don't have a fix
	// ------------------------------------
	update_GPS_light();

	if (print_telemetry){
	
		print_position();
		print_telemetry = false;
		
		if(invalid_location && current_loc.lat != 0){
			// Will help the plane recalc crosstrack
			// -------------------------------------
			reset_location();
		}
		if(GPS_flag == 0){
			// We countdown N number of good GPS fixes
			// so that the altitude is more accurate
			// -------------------------------------
			initialize_home();
			GPS_flag = -1;
			
		}else if(GPS_flag > 0){
			GPS_flag--;
		}
		gps_event();
		
		// save performance data to the EEPROM
		// -------------------------------------
		set_max_altitude_speed();
	}
}


void update_current_flight_mode(void)
{
	if (control_mode == FLY_BY_WIRE_A || control_mode == FLY_BY_WIRE_B) {
		// fake Navigation output using sticks
		nav_roll = ((ch1_in - ch1_trim) * (long)HEAD_MAX * REVERSE_ROLL) / (ch1_max - ch1_trim);
		nav_roll = constrain(nav_roll, -HEAD_MAX, HEAD_MAX); 

		// nav_pitch is the amount to adjust pitch due to altitude
		// We use PITCH_MIN because its magnitude is normally greater than PITCH_MAX
		nav_pitch = ((ch2_in - ch2_trim) * -1 * (long)PITCH_MIN *  REVERSE_PITCH) / (ch2_max - ch2_trim);
		nav_pitch = constrain(nav_pitch, PITCH_MIN, PITCH_MAX);

	} else if (control_mode == STABILIZE) {
		nav_roll = 0;
		nav_pitch = 0;
		
	} else if (control_mode == CIRCLE){
		// we have no GPS installed and have lost radio contact
		// or we just want to fly around in a gentle circle w/o GPS
		// ----------------------------------------------------
		nav_roll = HEAD_MAX / 3;
		
		if (failsafe == true){
			servo_throttle = THROTTLE_CRUISE;
		}
		
	} else if (control_mode == MANUAL){
		// set the outs for telemtry
		// -------------------------
		ch1_out = ch1_in;
		ch2_out = ch2_in;
		ch3_out = ch3_in;
	}
}

void check_for_takeoffComplete(void)
{
	if (takeoffComplete == 0) {
		if ((current_loc.alt > (home.alt + TAKE_OFF_ALT * 100)) && (GPS_flag < 0))  {
			takeoffComplete = 1;
		} else {
			if (control_mode > FLY_BY_WIRE_B) {
				nav_roll = 0;
				nav_pitch = TAKE_OFF_PITCH * 100;
			}
		}
	}
}
