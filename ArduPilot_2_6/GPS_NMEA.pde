#if GPS_PROTOCOL == 0

/****************************************************************
 Parsing stuff for NMEA
 ****************************************************************/
#define BUF_LEN 200

// GPS Pointers
char *token;
char *search = ",";
char *brkb, *pEnd;
char gps_buffer[BUF_LEN]; //The traditional buffer.

void init_gps(void)
{
	Serial.begin(9600);
	delay(1000);
	Serial.print(LOCOSYS_BAUD_RATE_38400);
	Serial.begin(THIRTY_EIGHT_K_BAUD);
	delay(500);
	Serial.print(LOCOSYS_REFRESH_RATE_250);
	delay(500);
	Serial.print(NMEA_OUTPUT_4HZ);
	delay(500);
	Serial.print(SBAS_OFF);

#if REMZIBI == 1
   init_remzibi_GPS(); 	
#endif

/* EM406 example init
	Serial.begin(4800); //Universal Sincronus Asyncronus Receiveing Transmiting 
	delay(1000);
	Serial.print(SIRF_BAUD_RATE_9600);
 
	Serial.begin(9600);
	delay(1000);
	
	Serial.print(GSV_OFF);
	Serial.print(GSA_OFF);
	
	#if USE_WAAS == 1
	Serial.print(WAAS_ON);
	#else
	Serial.print(WAAS_OFF);
	#endif*/
	
}

void decode_gps(void)
{
	const char head_rmc[]="GPRMC"; //GPS NMEA header to look for
	const char head_gga[]="GPGGA"; //GPS NMEA header to look for
	
	static unsigned long GPS_timer = 0; //used to turn off the LED if no data is received. 
	
	static byte unlock = 1; //some kind of event flag
	static byte checksum = 0; //the checksum generated
	static byte checksum_received = 0; //Checksum received
	static byte counter = 0; //general counter

	//Temporary variables for some tasks, specially used in the GPS parsing part (Look at the NMEA_Parser tab)
	unsigned long temp = 0;
	unsigned long temp2 = 0;
	unsigned long temp3 = 0;


	while(Serial.available() > 0)
	{
		if(unlock == 0)
		{
			gps_buffer[0] = Serial.read();//puts a byte in the buffer

			if(gps_buffer[0]=='$')//Verify if is the preamble $
			{
				counter 	= 0;
				checksum 	= 0;
				unlock		= 1;
			}
		} else {
			gps_buffer[counter] = Serial.read();

			if(gps_buffer[counter] == 0x0A)//Looks for \F
			{
				unlock = 0;

				if (strncmp (gps_buffer, head_rmc, 5) == 0)//looking for rmc head....
				{

					/*Generating and parsing received checksum, */
					for(int x=0; x<100; x++)
					{
						if(gps_buffer[x]=='*')
						{ 
							checksum_received = strtol(&gps_buffer[x + 1], NULL, 16);//Parsing received checksum...
							break; 
						}
						else
						{
							checksum ^= gps_buffer[x]; //XOR the received data... 
						}
					}

					if(checksum_received == checksum)//Checking checksum
					{
						/* Token will point to the data between comma "'", returns the data in the order received */
						/*THE GPRMC order is: UTC, UTC status , Lat, N/S indicator, Lon, E/W indicator, speed, course, date, mode, checksum*/
						token = strtok_r(gps_buffer, search, &brkb); //Contains the header GPRMC, not used

						token = strtok_r(NULL, search, &brkb); //UTC Time, not used
						//time=	atol (token);
						token = strtok_r(NULL, search, &brkb); //Valid UTC data? maybe not used... 


						//Longitude in degrees, decimal minutes. (ej. 4750.1234 degrees decimal minutes = 47.835390 decimal degrees)
						//Where 47 are degrees and 50 the minutes and .1234 the decimals of the minutes.
						//To convert to decimal degrees, devide the minutes by 60 (including decimals), 
						//Example: "50.1234/60=.835390", then add the degrees, ex: "47+.835390 = 47.835390" decimal degrees
						token = strtok_r(NULL, search, &brkb); //Contains Latitude in degrees decimal minutes... 

						//taking only degrees, and minutes without decimals, 
						//strtol stop parsing till reach the decimal point "."	result example 4750, eliminates .1234
						temp = strtol (token, &pEnd, 10);

						//takes only the decimals of the minutes
						//result example 1234. 
						temp2 = strtol (pEnd + 1, NULL, 10);

						//joining degrees, minutes, and the decimals of minute, now without the point...
						//Before was 4750.1234, now the result example is 47501234...
						temp3 = (temp * 10000) + (temp2);


						//modulo to leave only the decimal minutes, eliminating only the degrees.. 
						//Before was 47501234, the result example is 501234.
						temp3 = temp3 % 1000000;


						//Dividing to obtain only the de degrees, before was 4750 
						//The result example is 47 (4750/100 = 47)
						temp /= 100;

						//Joining everything and converting to float variable... 
						//First i convert the decimal minutes to degrees decimals stored in "temp3", example: 501234/600000 =.835390
						//Then i add the degrees stored in "temp" and add the result from the first step, example 47+.835390 = 47.835390 
						//The result is stored in "lat" variable... 
						//lat = temp + ((float)temp3 / 600000);
						current_loc.lat		= (temp * t7) + ((temp3 *100) / 6);

						token = strtok_r(NULL, search, &brkb); //lat, north or south?
						//If the char is equal to S (south), multiply the result by -1.. 
						if(*token == 'S'){
							current_loc.lat *= -1;
						}

						//This the same procedure use in lat, but now for Lon....
						token = strtok_r(NULL, search, &brkb);
						temp = strtol (token,&pEnd, 10); 
						temp2 = strtol (pEnd + 1, NULL, 10); 
						temp3 = (temp * 10000) + (temp2);
						temp3 = temp3%1000000; 
						temp/= 100;
						//lon = temp+((float)temp3/600000);
						current_loc.lng		= (temp * t7) + ((temp3 * 100) / 6);

						token = strtok_r(NULL, search, &brkb); //lon, east or west?
						if(*token == 'W'){
							current_loc.lng *= -1;
						}

						token = strtok_r(NULL, search, &brkb); //Speed overground?
						ground_speed = atoi(token) * 100;

						token = strtok_r(NULL, search, &brkb); //Course?
						ground_course = atoi(token) * 100;
						
						GPS_update |= GPS_POSITION; //Update the flag to indicate the new data has arrived. 

					}
					checksum = 0;
				}//End of the GPRMC parsing

				if (strncmp (gps_buffer, head_gga, 5) == 0)//now looking for GPGGA head....
				{
					/*Generating and parsing received checksum, */
					for(int x = 0; x<100; x++)
					{
						if(gps_buffer[x]=='*')
						{ 
							checksum_received = strtol(&gps_buffer[x + 1], NULL, 16);//Parsing received checksum...
							break; 
						}
						else
						{
							checksum^= gps_buffer[x]; //XOR the received data... 
						}
					}

					if(checksum_received== checksum)//Checking checksum
					{
						//strcpy(gps_GGA,gps_buffer);

						token = strtok_r(gps_buffer, search, &brkb);//GPGGA header, not used anymore
						token = strtok_r(NULL, search, &brkb);//UTC, not used!!
						token = strtok_r(NULL, search, &brkb);//lat, not used!!
						token = strtok_r(NULL, search, &brkb);//north/south, nope...
						token = strtok_r(NULL, search, &brkb);//lon, not used!!
						token = strtok_r(NULL, search, &brkb);//wets/east, nope
						token = strtok_r(NULL, search, &brkb);//Position fix, used!!
						GPS_fix = atoi(token); 
						if(GPS_fix >= 1){
							GPS_fix = VALID_GPS;
							print_telemetry = true;
						}else{
							GPS_fix = BAD_GPS;
						}
						token = strtok_r(NULL, search, &brkb); //sats in use!! Nein...
						token = strtok_r(NULL, search, &brkb);//HDOP, not needed
						token = strtok_r(NULL, search, &brkb);//ALTITUDE, is the only meaning of this string.. in meters of course. 
						//alt_MSL = atoi(token);
						//if(alt_MSL<0){
						//	alt_MSL = 0;
						//}
						current_loc.alt = (long)abs(atoi(token)) * 100;
						
						if(GPS_fix== VALID_GPS) digitalWrite(12, HIGH); //Status LED...
						else digitalWrite(12, LOW);
						
						GPS_update |= GPS_HEADING; //Update the flag to indicate the new data has arrived.
					}
					checksum = 0; //Restarting the checksum
				}

				for(int a = 0; a<= counter; a++)//restarting the buffer
				{
					gps_buffer[a]= 0;
				} 
				counter = 0; //Restarting the counter
				GPS_timer = DIYmillis(); //Restarting timer...
			}
			else
			{
				counter++; //Incrementing counter
				if (counter >= 200)
				{
					//Serial.flush();
					counter = 0;
					checksum = 0;
					unlock = 0;
				}
			}
		}
	}
	
	if(DIYmillis() - GPS_timer > 2000){
		digitalWrite(12, LOW);	//If we don't receive any byte in two seconds turn off gps fix LED... 
		if(GPS_fix != FAILED_GPS){
			GPS_fix = BAD_GPS;
		}
		GPS_update = GPS_NONE;
		
		if(DIYmillis() - GPS_timer > 20000){
			invalid_location = true;
			GPS_fix = FAILED_GPS;
			GPS_timer = DIYmillis();
			Serial.println("no GPS, last 20s");
		}
	}
}
#endif

