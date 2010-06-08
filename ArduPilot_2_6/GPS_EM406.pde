#if GPS_PROTOCOL == 1

#define BUF_LEN 100

// The input buffer
char gps_buffer[BUF_LEN]={
	0x24,0x50,0x53,0x52,0x46,0x31,0x30,0x30,0x2C,0x30,0x2C,0x35,0x37,0x36,0x30,0x30,0x2C,0x38,0x2C,0x31,0x2C,0x30,0x2A,0x33,0x37,0x0D,0x0A};	

// Used to configure Sirf GPS
const byte gps_ender[]={0xB0,0xB3};	

/****************************************************************
 Parsing stuff for SIRF binary protocol. 
 ****************************************************************/
void init_gps(void)
{
	Serial.println("init_gps");
	change_to_sirf_protocol();
	delay(100);//Waits fot the GPS to start_UP
	configure_gps();//Function to configure GPS, to output only the desired msg's
}

void decode_gps(void)
{
	static unsigned long GPS_timer = 0;
	static byte gps_counter = 0; //Another gps counter for the buffer
	static byte GPS_step = 0;
	static byte gps_ok = 0;//Counter to verify the reciving info
	const byte read_gps_header[]={0xA0,0xA2,0x00,0x5B,0x29};//Used to verify the payload msg header

	if(Serial.available() > 0)//Ok, let me see, the buffer is empty?
	{
		while(Serial.available() < 5){ }
		switch(GPS_step) {
			case 0: //This case will verify the header, to know when the payload will begin 
				while(Serial.available() > 0)	//Loop if data available
				{
					if(Serial.read() == read_gps_header[gps_ok]){ //Checking if the head bytes are equal..
						//if yes increment 1
						gps_ok++; 
					}else{ 
						//Otherwise restart.
						gps_ok = 0; 
					}
					if(gps_ok >= 5) {
						//Ohh 5 bytes are correct, that means jump to the next step, and break the loop
						gps_ok = 0;
						GPS_step++;
						break;
					}
				}
				break; 
			case 1: //Will receive all the payload and join the received bytes... 
				while(Serial.available() < 92){
				}
				gps_counter = 0;
				memset(gps_buffer,0,sizeof(gps_buffer));
				
				while(Serial.available() > 0){
					//Read data and store it in the temp buffer
					byte b1 = Serial.read();
					byte b2 = gps_buffer[gps_counter-1];
					// gps_ender[]={0xB0,0xB3};	

					if((b1 == gps_ender[1]) && (b2 == gps_ender[0])){
						GPS_step = 0;
						gps_counter = 0;
						GPS_fix = gps_buffer[1];
						
						if(GPS_fix == VALID_GPS){
							// GPS signal is error free
							// ------------------------
							digitalWrite(12,HIGH);
							GPS_timer = DIYmillis();
							GPS_update = GPS_BOTH;

							print_telemetry = true;
							
							//Parse the data
							GPS_join_data(); 

						} else {
							// GPS has returned an error code
							// ------------------------------
							GPS_fix = BAD_GPS;
							GPS_update = GPS_NONE;
							digitalWrite(12,LOW);
						}
						
						break;
					}else{
						gps_buffer[gps_counter] = b1;
						gps_counter++;
						
						if (gps_counter >= BUF_LEN){
							Serial.flush();
							break;
						}
					}
				}
				break;
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

void GPS_join_data(void)
{
	// Read bytes and combine them with Unions
	// ---------------------------------------
	byte j = 22;
	longUnion.byte[3] 	= gps_buffer[j++];
	longUnion.byte[2] 	= gps_buffer[j++];
	longUnion.byte[1] 	= gps_buffer[j++];
	longUnion.byte[0] 	= gps_buffer[j++];
	current_loc.lat		= longUnion.dword;		//Y = lat * 10,000,000


	longUnion.byte[3] 	= gps_buffer[j++];
	longUnion.byte[2] 	= gps_buffer[j++];
	longUnion.byte[1] 	= gps_buffer[j++];
	longUnion.byte[0] 	= gps_buffer[j++];
	current_loc.lng 	= longUnion.dword;		// X = long * 10,000,000

	j = 34;
	longUnion.byte[3] 	= gps_buffer[j++];
	longUnion.byte[2] 	= gps_buffer[j++];
	longUnion.byte[1] 	= gps_buffer[j++];
	longUnion.byte[0] 	= gps_buffer[j++];	
	current_loc.alt 	= longUnion.dword;		// alt in meters * 100

	j = 39;
	intUnion.byte[1] 	= gps_buffer[j++];
	intUnion.byte[0] 	= gps_buffer[j++];
	ground_speed 		= intUnion.word;		// meters/second * 100
	

	//iTOW
	
	if(ground_speed >= 50){
		//Only updates data if we are really moving... 
		intUnion.byte[1] 	= gps_buffer[j++];
		intUnion.byte[0] 	= gps_buffer[j++];
		ground_course 		= intUnion.word;		// degrees * 100
		ground_course 		= abs(ground_course);	//The GPS has a BUG sometimes give you the correct value but negative, weird!! 
	}else{
		//ground_speed 	= 0;
		//ground_course = 0;
	}

	j = 45;
	intUnion.byte[1] 	= gps_buffer[j++];
	intUnion.byte[0] 	= gps_buffer[j++];
	climb_rate 			= intUnion.word;	//m/s * 100

	// We can't navigate until the GPS has output data
	// We can't read GPS while in FBW. Therfore we need to avoid navigating with Old GPS data 
	// or we might send the plan into the ground
	// ------------------------
	if(current_loc.lat == 0){
		GPS_update = GPS_NONE;
		GPS_fix = BAD_GPS;
	}
	
	// clear buffer
	// -------------
	memset(gps_buffer,0,sizeof(gps_buffer));
}


void configure_gps(void)
{
	const byte gps_header[]={
		0xA0,0xA2,0x00,0x08,0xA6,0x00			};//Used to configure Sirf GPS
	const byte gps_payload[]={
		0x02,0x04,0x07,0x09,0x1B			};//Used to configure Sirf GPS
	const byte gps_checksum[]={
		0xA8,0xAA,0xAD,0xAF,0xC1			};//Used to configure Sirf GPS
	const byte cero = 0x00;//Used to configure Sirf GPS

	for(int z=0; z<2; z++)
	{
		for(int x=0; x<5; x++)//Print all messages to setup GPS
		{
			for(int y=0; y<6; y++)
			{
				Serial.print(byte(gps_header[y]));//Prints the msg header, is the same header for all msg..	
			} 
			Serial.print(byte(gps_payload[x]));//Prints the payload, is not the same for every msg
			for(int y=0; y<6; y++)
			{
				Serial.print(byte(cero)); //Prints 6 zeros
			} 
			Serial.print(byte(gps_checksum[x])); //Print the Checksum
			Serial.print(byte(gps_ender[0]));	//Print the Ender of the string, is same on all msg's. 
			Serial.print(byte(gps_ender[1]));	//ender	
		}
	}	
}

void change_to_sirf_protocol(void)
{
	Serial.begin(4800); //First try in 4800
	delay(300);
	for (byte x=0; x<=28; x++)
	{
		Serial.print(byte(gps_buffer[x]));//Sending special bytes declared at the beginning 
	}	
	delay(300);
	Serial.begin(9600); //Then try in 9600 
	delay(300);
	for (byte x=0; x<=28; x++)
	{
		Serial.print(byte(gps_buffer[x]));
	}
	Serial.begin(FIFTY_SEVEN_K_BAUD); //Universal Sincronus Asyncronus Receiveing Transmiting
}

#endif

