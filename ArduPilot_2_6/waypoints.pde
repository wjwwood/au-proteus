void save_wp_index()
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x0A, wp_index);
}

void return_to_launch()
{
	// home is WP 0
	// ------------
	wp_index = 0;

	// Loads WP from Memory
	// --------------------
	load_waypoint();

	// Altitude to hold over home
	// Set by configuration tool
	// -------------------------
	if (config_tool_options & HOLD_ALT_ABOVE_HOME){
 		next_WP.alt = current_loc.alt;
 	}else{
		eeprom_busy_wait();
		int hold_alt_above_home = eeprom_read_word((uint16_t *)	0x16);
		next_WP.alt = home.alt + ((long)hold_alt_above_home * 100);
	}
}

void reached_waypoint()
{
	if (control_mode == RTL){
		set_mode(LOITER);
		
	}else if (control_mode == AUTO){
		// load the next waypoint
		// ----------------------
		wp_index++;
		// are we looping or returning home?
		if(wp_index > wp_total){
			eeprom_busy_wait();
			byte loop_waypoints = eeprom_read_byte((uint8_t *)	0x3E6);
			
			if (loop_waypoints > 0){
				Serial.println("XXX \t Looping, going to waypoint 1 \t ***");

				waypoint_event(EVENT_LOOP);

				wp_index = 1; // first real waypoint
				waypoint_event(EVENT_SET_NEW_WAYPOINT_INDEX);
				loop_waypoints--;
				waypoint_event(EVENT_LOOP);
				
				// save loop_waypoints counter
				eeprom_busy_wait();
				eeprom_write_byte((uint8_t * )0x3E6, loop_waypoints);
				
			}else {
				Serial.println("XXX \t Done, Heading home \t ***");
				set_mode(RTL);
				// nothing more to do here
				return;
			}
		}else{
			
			Serial.print(iTOW, DEC);
			Serial.print(" Moving on to waypoint: ");
			Serial.println(wp_index,DEC);
		}
		
		// notify user of new index selection
		// -------------------------------------
		waypoint_event(EVENT_SET_NEW_WAYPOINT_INDEX);
		
#ifdef XBEE_READ
		// XXX: mod to remove arbitrary waypoint
		fakeWP = false;
#endif

		// load next WP
		// ------------
		load_waypoint();
	}
}

// run this whenever the wp_index changes
// -------------------------------------
void load_waypoint()
{
	// Save current waypoint index to EEPROM
	// -------------------------------------
	save_wp_index();

	// copy the current WP into the OldWP slot
	// ---------------------------------------
	prev_WP = current_loc;

	// Load the next_WP slot
	// ---------------------
	next_WP = get_loc_with_index(wp_index);

	// offset the altitude relative to home position
	// ---------------------------------------------
	next_WP.alt += home.alt;

	// let them know we have loaded the WP
	// -----------------------------------
	waypoint_event(EVENT_LOADED_WAYPOINT);
	
	// do this whenever Old and new WP's change
	// ---------------------------------------------
	precalc_waypoint_distance();
	crosstrack_bearing  =  get_bearing(&current_loc, &next_WP);
}


// run this at setup on the ground
// -------------------------------
void initialize_home()
{
	//Serial.println("initialize_home");
	#if GPS_PROTOCOL == 5
		//init_test_location();
	#endif

	// Copy our current location to home
	// ---------------------------------
	home = current_loc;
	set_loc_with_index(home, 0);

	prev_WP = home;
	
	// Set the current WP index to 1 (first WP)
	// ----------------------------------------
	wp_index = 1;
}

struct Location set_loc_with_index(struct Location temp, int i)
{
	temp.lat = temp.lat/10;
	temp.lng = temp.lng/10;		// lat and long stored as * 1,000,000
	temp.alt = temp.alt/100; 	// altitude is stored as meters 
	
	if (i == 0){

		// Save Home location to EEPROM
		// ----------------------------
		eeprom_busy_wait();
		eeprom_write_dword((uint32_t *)0x0E, temp.lat);
		eeprom_busy_wait();
		eeprom_write_dword((uint32_t *)0x12, temp.lng);
		eeprom_busy_wait();
		eeprom_write_word((uint16_t *)0x0C, (int)temp.alt);
	
	}else{
	
		/* this is not being used right now
		// --------------------------------
		long mem_position = (long)(WP_START_BYTE + (i-1) * WP_10_BYTES);
    	eeprom_busy_wait();		
		eeprom_write_dword((uint32_t *)	mem_position, temp.lat);
		eeprom_busy_wait();
		mem_position += 4;
		eeprom_write_dword((uint32_t *)	mem_position, temp.lng);
		eeprom_busy_wait();
		mem_position += 4;
		eeprom_write_word((uint16_t *)	mem_position, (int)temp.alt);
		*/
	}
}


struct Location get_loc_with_index(int i)
{
	struct Location temp;
	long mem_position;

	// Find out proper location in memory by using the start_byte position + the index
	// --------------------------------------------------------------------------------
	if (i == 0) {
		// read home position 
		eeprom_busy_wait();
		temp.lat = (long)eeprom_read_dword((uint32_t*)0x0E);eeprom_busy_wait();
		temp.lng = (long)eeprom_read_dword((uint32_t*)0x12);eeprom_busy_wait();
		temp.alt = 0;
		temp.alt = (long)eeprom_read_word((uint16_t*)0x0C);

		temp.lat *= 10;
		temp.lng *= 10;
		temp.alt *= 100;
		return temp;
		
	}else{
		// read WP position 
		mem_position = (long)(WP_START_BYTE + (i-1) * WP_10_BYTES);
		eeprom_busy_wait();
		temp.lat = (long)eeprom_read_dword((uint32_t*)mem_position);
		mem_position += 4;
		eeprom_busy_wait();
		temp.lng = (long)eeprom_read_dword((uint32_t*)mem_position);
		mem_position += 4;
		temp.alt = 0;
		eeprom_busy_wait();
		temp.alt = (long)eeprom_read_word((uint16_t*)mem_position);
		
		temp.lat *= 10;
		temp.lng *= 10;
		temp.alt *= 100;
		/*
		if(wp_mode == REL_WP || abs(temp.lat) < 10000 ){
			wp_mode = REL_WP;
			temp.lat += home.lat;
			temp.lng += home.lng;
		}
		*/
		
		return temp;
	}
}

void readPoints()
{
    for (byte i = 0; i < wp_total; i++){
    
    	struct Location tmp = get_loc_with_index(i);
    			
		Serial.print("waypoint #");
		Serial.print(i);
		Serial.print("\t");
		Serial.print(tmp.lat,DEC);
		Serial.print("\t");
		Serial.print(tmp.lng,DEC);
		Serial.print("\t");
		Serial.println(tmp.alt,DEC);
	}
}

// reset Current Location to be the originating point
// This allows us to start navigating from an arbitrary point if 
// the system is in RTL
// --------------------
void reset_location()
{
	invalid_location = false;
	print_position();
	
	if (control_mode == LOITER){
		next_WP = current_loc;
		prev_WP = current_loc;
	}else{
		// copy location into previous waypoint
		prev_WP = current_loc;
	}	
	// do our precalcs
	precalc_waypoint_distance();
}

byte get_waypiont_mode(void)
{
	eeprom_busy_wait();
	return eeprom_read_byte((uint8_t*)0x3E7);
}

// Precalc for navigation algorithms
// called on each WP change
// ---------------------------------
void precalc_waypoint_distance(void)
{
	// this is handy for the groundstation
	wp_totalDistance 	= getDistance(&current_loc, &next_WP);

	// this is used to offset the shrinking longitude as we go towards the poles	
	float rads = (abs(next_WP.lat)/t7) * 0.0174532925;
	//377,173,810 / 10,000,000 = 37.717381 * 0.0174532925 = 0.658292482926943		
	//scaleLongDown = cos(rads);
	scaleLongUp = 1.0f/cos(rads);

	// set a new crosstrack bearing
	// ----------------------------
	reset_crosstrack();
	
	// output the new WP information to the Ground Station
	// ---------------------------------------------------
	#if GCS_PROTOCOL == 5
	print_new_wp_info();
	#endif
}

void reset_crosstrack()
{
	crosstrack_bearing 	= get_bearing(&current_loc, &next_WP);	// Used for track following
}

// utility to reset WP index to 0 se we can restart mission
void reset_waypoint_index(void){
	//Serial.println("reset_waypoint_index");
	wp_index = 1;	// first WP
	load_waypoint();
}


#ifdef XBEE_READ
// run this whenever we have a "fake" WP, due to collision avoidance
// -------------------------------------
void load_waypoint(struct Location *wp)
{
	// flush our temp waypoint if receiving a {1,2,3}
	if (wp->lat == 10000000L && wp->lng == 20000000L && wp->alt == 300L) {
		Serial.println("Flushing arb. waypoint");
		reached_waypoint();
		return;
	}
		
	// copy the current WP into the OldWP slot
	// ---------------------------------------
	prev_WP = current_loc;

	// Load the next_WP slot
	// ---------------------
	next_WP = *wp;

	// Decrement wp_index so that once this "fake" WP is reached,
	// we'll go to where we were supposed to originally
	wp_index--;
	fakeWP = true;

	// offset the altitude relative to home position
	// ---------------------------------------------
	next_WP.alt += home.alt;

	// let them know we have loaded the WP
	// -----------------------------------
	waypoint_event(EVENT_LOADED_WAYPOINT);
	
	// do this whenever Old and new WP's change
	// ---------------------------------------------
	precalc_waypoint_distance();
	crosstrack_bearing  =  get_bearing(&current_loc, &next_WP);
}
#endif
