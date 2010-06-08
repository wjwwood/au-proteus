#if GPS_PROTOCOL == 5
// The input buffer
#define BUF_LEN 30
char gps_buffer[BUF_LEN];
float dlat,dlng;

void decode_gps(void)
{
	static unsigned long GPS_timer = 0;
	
	//testing 1hz simulation
	if((millis() - GPS_timer) > 1000) {
		readCommands();
		
		#if THROTTLE_IN == 0
			ground_speed = (airspeed_current * 1340) / 30;
		#else
			ground_speed = ((float)(ch3_in - 1000) * 2000) / 1000;
		#endif
				
		if(GPS_fix == VALID_GPS){
			GPS_timer = millis(); //Restarting timer...
			digitalWrite(12,HIGH);
			GPS_update 				= GPS_BOTH;	
			print_telemetry			= true;
		}else{
			digitalWrite(12,LOW);
		}
		
		if((millis() - GPS_timer) > 2000){
			digitalWrite(12, LOW);	//If we don't receive any byte in two seconds turn off gps fix LED... 
			if(GPS_fix != FAILED_GPS){
				GPS_fix = BAD_GPS;
			}
			GPS_update = GPS_NONE;
	
			if((millis() - GPS_timer) > 20000){
				invalid_location = true;
				GPS_fix = FAILED_GPS;
				GPS_timer = millis();
				Serial.println("no GPS, last 20s");
				//reinit the GPS Modules
			}
		}

	}
}


void init_gps(void)
{
	pinMode(12, OUTPUT);//Status led
	Serial.begin(THIRTY_EIGHT_K_BAUD); //Universal Sincronus Asyncronus Receiveing Transmiting 
	setCommandMux();
	GPS_update 	= GPS_NONE;
	GPS_fix 	= BAD_GPS;

	//fast_init_gps();
	//wait_for_GPS_fix();
}



void init_test_location(void)
{
	//Serial.println("init_test_location");
	GPS_fix 		= VALID_GPS;	
	GPS_update 		= GPS_BOTH;
	print_telemetry = true;
	
	current_loc = get_loc_with_index(1);
	current_loc.lat -= 9500;
	current_loc.lng -= 9500;
}


void readCommands(void)
{
	static byte bufferPointer = 0;
	static byte header[2];
	const byte read_GS_header[] 	= {0x21, 0x21}; //!! Used to verify the payload msg header

	if(Serial.available()){
	
		Serial.println("Serial.available");
		bufferPointer = 0;
		
		byte tmp = Serial.read();
		header[0] = tmp;
		Serial.println(tmp);
		
		tmp = Serial.read();
		header[1] = tmp;
		Serial.println(tmp);
		Serial.println("Serial.available");
		
		//header[0] = Serial.read();
		//header[1] = Serial.read();
		
		byte test = 0;

		if(header[0] == read_GS_header[0]) test++;
		if(header[1] == read_GS_header[1]) test++;
		
		if(test == 2){
			// Block until we read full command 
			// --------------------------------
			delay(20);
			byte incoming_val = 0;

			// Ground Station communication 
			// ----------------------------
			while(Serial.available() > 0) 
			{
				incoming_val = Serial.read();		 

				if (incoming_val != 13 && incoming_val != 10 ) {	 
					gps_buffer[bufferPointer++] = incoming_val;	
				}

				if(bufferPointer >= BUF_LEN){
					Serial.println("Big buffer overrun");
					bufferPointer = 0;
					gps_buffer[0] = 1;
					Serial.flush();
					memset(gps_buffer,0,sizeof(gps_buffer));
					return;
				}
			}
			parseCommand(gps_buffer);
			
			// clear buffer of old data
			// ------------------------
			memset(gps_buffer,0,sizeof(gps_buffer));

		}else{
			Serial.flush();
		}
	}
}

// Commands can be sent as !!a:100|b:200|c:1
// -----------------------------------------
void parseCommand(char *buffer)
{
	Serial.println("got cmd ");
	char *token, *saveptr1, *saveptr2;
	
	for (int j = 1;; j++, buffer = NULL) {
		token = strtok_r(buffer, "|", &saveptr1);
		if (token == NULL) break;	
		
		char * cmd 		= strtok_r(token, ":", &saveptr2);
		long value		= strtol(strtok_r (NULL,":", &saveptr2), NULL,0);
		
		///*
		Serial.print("cmd ");
		Serial.print(cmd[0]);
		Serial.print("\tval ");
		Serial.println(value);
		Serial.println("");
		//*/
		///*
		switch(cmd[0]){
		
			case 'h':
			init_test_location();
			//Serial.println("init_test_location");
			break;

			case 'd':
			GPS_fix = BAD_GPS;
			Serial.println("disable-GPS");
			break;
			
			case 'g':
			GPS_fix = VALID_GPS;
			Serial.println("enable-GPS");
			break;
			
			case 'w':
			while (value >0){
				demo_servos();
				delay(200);
				value--;
			}
			break;
	
			case 'r': // return Home
			return_to_launch();
			break;

			case 'l': // return Home
			set_mode(LOITER);
			break;

			case 's':
			reached_waypoint();
			load_waypoint();
			break;
			
			case 'z'://reset
			wp_index = 1;
			load_waypoint();
			break;
			

		}
		//*/
	}
}


void navigate_sim()
{
	
	if(GPS_update & GPS_BOTH)
	{
		/* 
		this is what the GPS provides:
		ground_course	- 0 to 359 degrees *100
		ground_speed  	- m/s * 100
		climb_rate		- m/s * 100
		*/

		// guess the climb rate
		// --------------------
		if(pitch_sensor >= 0){
			climb_rate = (pitch_sensor * CLIMBRATE_UP * (long)deltaMiliSeconds) / 90000L;
		}else{
			climb_rate = (pitch_sensor * CLIMBRATE_DOWN * (long)deltaMiliSeconds) / 90000L;
		}
		
		dlat 	= dlng = 0;
		est_loc = current_loc;

	}else{
	
		// Dead Reckon:
		// Estimate the location of the aircraft
		// -------------------------------------
		float pb_Rad 		= (float)ground_course * .0001745;
		float dist 			= ((float)(ground_speed * deltaMiliSeconds)) / 1000;
		dlat 				+= cos(pb_Rad) * dist;
		dlng 				+= sin(pb_Rad) * dist;	
		current_loc.lat 	= est_loc.lat + dlat;	// latitude = Y part
		current_loc.lng 	= est_loc.lng + (dlng * scaleLongUp);	// Longitude = X part (scaled)
			// use climb_rate from IR Sensors
		current_loc.alt 	= est_loc.alt + climb_rate;
		
		// clamp the roll sensor so wildness doesn't ensue
		// ----------------------------------------------
		long roll_sensorClamp = constrain(roll_sensor, -4500, 4500);
		
		// run simulation to arrive at intermediate values
		// -----------------------------------------------
		est_turn_rate = (roll_sensorClamp * (long)TURNRATE * (long)deltaMiliSeconds) / 90000L;
		// 4500 * 130 * 1000 / 90000 = 6500
				
		// Integrate the turn rate guess - GPS will overwrite this val 
		// -----------------------------------------------------------
		ground_course += est_turn_rate;

		// Save turn rate for the print function
		// -------------------------------------
		est_turn_rate = -(roll_sensorClamp * (long)TURNRATE) / 90L;
		// 4500 * 130 / 90 = 6500

		// wrap ground_course values
		// -------------------------
		if (ground_course > 36000)	ground_course -= 36000;
		if (ground_course < 0) 		ground_course += 36000;
		
	}
}
#endif

