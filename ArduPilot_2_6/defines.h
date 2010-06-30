// For Alpha Testers only
// ----------------------


// Do not Change below this line
// -----------------------------
#define FIFTY_SEVEN_K_BAUD 57600
#define THIRTY_EIGHT_K_BAUD 38400
#define WP_10_BYTES 10
#define IR_MAX_FIX .88

// PID cases
// ----------------
#define CASE_SERVO_ROLL 0
#define CASE_SERVO_PITCH 1
#define CASE_NAV_ROLL 2
#define CASE_NAV_PITCH 3
#define CASE_TE_THROTTLE 4
#define CASE_FBW_THROTTLE 5

// Auto Pilot modes
// ----------------
#define MANUAL 0
#define CIRCLE 1		   // When flying sans GPS, and we loose the radio, just circle
#define STABILIZE 2
#define FLY_BY_WIRE_A 3    // Fly By Wire A has left stick horizontal => desired roll angle, left stick vertical => desired pitch angle, right stick vertical = manual throttle
#define FLY_BY_WIRE_B 4    // Fly By Wire B has left stick horizontal => desired roll angle, left stick vertical => desired pitch angle, right stick vertical => desired airspeed
                           // Fly By Wire B = Fly By Wire A if you have AIRSPEED_SENSOR 0
#define AUTO 6
#define RTL 7
#define LOITER 8

// Waypoint Modes
// ----------------
#define ABS_WP 0
#define REL_WP 1

// Events
// ------
#define EVENT_WILL_REACH_WAYPOINT 1
#define EVENT_SET_NEW_WAYPOINT_INDEX 2
#define EVENT_LOADED_WAYPOINT 3
#define EVENT_LOOP 4

// GPS flags
// ---------
//GPS_update
#define GPS_NONE 0
#define GPS_POSITION 1
#define GPS_HEADING 2
#define GPS_BOTH 3
#define GPS_IMU 4
#define GPS_IMU_ERROR 8

//GPS_fix
#define VALID_GPS 0x00
#define BAD_GPS 0x01
#define FAILED_GPS 0x03



#define WP_START_BYTE 0x18		// where in memory home WP is stored + all other WP
#define MPDEGREE 0.000009
#define BATTERY_VOLTAGE(x) (x*(INPUT_VOLTAGE/1024.0))/0.000294117



#define HOLD_ALT_ABOVE_HOME 8 // bitmask value


//GPS SIRF configuration strings... 
#define SIRF_BAUD_RATE_4800    "$PSRF100,1,4800,8,1,0*0E\r\n"
#define SIRF_BAUD_RATE_9600    "$PSRF100,1,9600,8,1,0*0D\r\n"
#define SIRF_BAUD_RATE_19200    "$PSRF100,1,19200,8,1,0*38\r\n"
#define SIRF_BAUD_RATE_38400    "$PSRF100,1,38400,8,1,0*3D\r\n"  
#define SIRF_BAUD_RATE_57600    "$PSRF100,1,57600,8,1,0*36\r\n"
#define GSA_ON   "$PSRF103,2,0,1,1*27\r\n"   // enable GSA
#define GSA_OFF  "$PSRF103,2,0,0,1*26\r\n"   // disable GSA
#define GSV_ON   "$PSRF103,3,0,1,1*26\r\n"  // enable GSV
#define GSV_OFF  "$PSRF103,3,0,0,1*27\r\n"  // disable GSV
#define USE_WAAS   1     //1 = Enable, 0 = Disable, good in USA, slower FIX... 
#define WAAS_ON    "$PSRF151,1*3F\r\n"       // enable WAAS
#define WAAS_OFF   "$PSRF151,0*3E\r\n"       // disable WAAS

//GPS Locosys configuration strings...
#define USE_SBAS 0
#define SBAS_ON "$PMTK313,1*2E\r\n"
#define SBAS_OFF "$PMTK313,0*2F\r\n"

#define NMEA_OUTPUT_5HZ "$PMTK314,0,5,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n" //Set GGA and RMC to 5HZ  
#define NMEA_OUTPUT_4HZ "$PMTK314,0,4,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n" //Set GGA and RMC to 4HZ 
#define NMEA_OUTPUT_3HZ "$PMTK314,0,3,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n" //Set GGA and RMC to 3HZ 
#define NMEA_OUTPUT_2HZ "$PMTK314,0,2,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n" //Set GGA and RMC to 2HZ 
#define NMEA_OUTPUT_1HZ "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n" //Set GGA and RMC to 1HZ

#define LOCOSYS_REFRESH_RATE_200 "$PMTK220,200*2C" //200 milliseconds 
#define LOCOSYS_REFRESH_RATE_250 "$PMTK220,250*29\r\n" //250 milliseconds

#define LOCOSYS_BAUD_RATE_4800 "$PMTK251,4800*14\r\n"
#define LOCOSYS_BAUD_RATE_9600 "$PMTK251,9600*17\r\n"
#define LOCOSYS_BAUD_RATE_19200 "$PMTK251,19200*22\r\n"
#define LOCOSYS_BAUD_RATE_38400 "$PMTK251,38400*27\r\n"

