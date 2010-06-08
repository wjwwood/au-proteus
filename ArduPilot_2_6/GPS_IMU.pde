#if GPS_PROTOCOL == 3
	// Performance Monitoring variables
	// Data collected and reported for ~1 minute intervals
	//int IMU_mainLoop_count = 0;				//Main loop cycles since last report

	byte IMU_buffer[30];
	byte payload_length	= 0;
	byte payload_counter	= 0;

	//IMU Checksum
	byte ck_a = 0;
	byte ck_b = 0;
	byte IMU_ck_a = 0;
	byte IMU_ck_b = 0;

/****************************************************************
 * Here you have all the stuff for data reception from the IMU_GPS
 ****************************************************************/

/*	GPS_update bit flags -
	 - 0x01 bit = gps lat/lon data received
	 - 0x02 bit = gps alt and speed data received
	 - 0x04 bit = IMU data received
	 - 0x08 bit = PROBLEM - No IMU data last second!

#define GPS_NONE 0
#define GPS_POSITION 1
#define GPS_HEADING 2
#define GPS_BOTH 3
#define GPS_IMU 4
#define GPS_IMU_ERROR 8

*/

void init_gps(void)
{
	Serial.begin(THIRTY_EIGHT_K_BAUD); //Universal Sincronus Asyncronus Receiveing Transmiting 
	GPS_update 	= GPS_NONE;
	GPS_fix 	= BAD_GPS;
}

/*
IMU Message format
Byte(s)		 Value
0-3		 Header "DIYd"
4						Payload length	= 6
5						Message ID = 2
6,7			roll		Integer (degrees*100)
8,9			pitch		Integer (degrees*100)
10,11		yaw			Integer (degrees*100)
12,13					checksum


GPS Message format
Byte(s)		 Value
0-3		 Header "DIYd"
4								Payload length = 14
5								Message ID = 3
6-9			longitude			Integer (value*10**7)
10-13		latitude			Integer (value*10**7)
14,15		altitude			Integer (meters*10)
16,17		gps speed			Integer (M/S*100)
18,19		gps course			not used
20,21		checksum
*/

void decode_gps(void)
{
	static unsigned long IMU_timer = 0; //used to set PROBLEM flag if no data is received. 
	static unsigned long GPS_timer = 0;
	static byte IMU_step = 0;
	int numc = 0;
	byte data;
	static byte message_num = 0;

	numc = Serial.available();
	if (numc > 0){
		for (int i=0;i<numc;i++){	// Process bytes received
			data = Serial.read();
			switch(IMU_step){	 	//Normally we start from zero. This is a state machine

			case 0:	
				if(data == 0x44){
					IMU_step++; //First byte of data packet header is correct, so jump to the next step
				}
				//}else{
					//Serial.println("IMU parser Case 0 fail");	 // This line for debugging only
				//}
				break; 

			case 1:	
				if(data == 0x49){
					 IMU_step++;	//Second byte of data packet header is correct
				}else {	
					// This line for debugging only
					//Serial.println("IMU parser Case 1 fail");	 
					IMU_step=0;		 //Second byte is not correct so restart to step zero and try again.	
				}	 
				break;

			case 2:	
				if(data == 0x59){
					 IMU_step++;	//Third byte of data packet header is correct
				}else {
					//Serial.println("IMU parser Case 2 fail");	 // This line for debugging only
					IMU_step=0;		 //Third byte is not correct so restart to step zero and try again.
				}		 
				break;

			case 3:	
				if(data == 0x64){ 
					 IMU_step++;	//Fourth byte of data packet header is correct, Header complete
				} else {
					//Serial.println("IMU parser Case 3 fail");	 // This line for debugging only
					IMU_step=0;		 //Fourth byte is not correct so restart to step zero and try again.
				}		 
				break;

			case 4:	
				payload_length = data;
				checksum(payload_length);
				IMU_step++;		
				if (payload_length>28){
					IMU_step=0;	 //Bad data, so restart to step zero and try again.		 
					payload_counter=0;
					ck_a=0;
					ck_b=0;
					imu_payload_error_count++;
				} 
				break;

			case 5:	
					message_num = data;
					checksum(data);
					IMU_step++;		 
				break;
		 
			case 6:				 // Payload data read...
				// We stay in this state until we reach the payload_length
				IMU_buffer[payload_counter] = data;
				checksum(data);
				payload_counter++;
				if (payload_counter >= payload_length) { 
					IMU_step++; 
				}
				break;
			case 7:
				IMU_ck_a=data;	 // First checksum byte
				IMU_step++;
				break;
			case 8:
				IMU_ck_b=data;	 // Second checksum byte

				// We end the IMU/GPS read...
				// Verify the received checksum with the generated checksum.. 
				if((ck_a == IMU_ck_a) && (ck_b == IMU_ck_b)) {
					if (message_num == 0x02) {
						IMU_join_data();
						IMU_timer = DIYmillis();
					} else if (message_num == 0x03) {
						GPS_join_data();
						GPS_timer = DIYmillis();
					} else if (message_num == 0x0a) {
						PERF_join_data();
					} else {
						Serial.print("Invalid message number = ");
						Serial.println(message_num,DEC);
					}
				} else {
					//Serial.println("Checksum error");	//bad checksum
					imu_checksum_error_count++;
				} 						 
				// Variable initialization
				IMU_step = 0;
				payload_counter = 0;
				ck_a = 0;
				ck_b = 0;
				IMU_timer = DIYmillis(); //Restarting timer...
				break;
			}
		}// End for...
	}

	if((DIYmillis() - IMU_timer) > 500){	//If we don't receive IMU data in 1/2 second, set flag
		digitalWrite(12, LOW);
		GPS_update = GPS_IMU_ERROR;
	}
	
	if((DIYmillis() - GPS_timer) > 2000){
		digitalWrite(12, LOW);	//If we don't receive any byte in two seconds turn off gps fix LED... 
		if(GPS_fix != FAILED_GPS){
			GPS_fix = BAD_GPS;
		}

		if((DIYmillis() - GPS_timer) > 10000){
			invalid_location = true;
			GPS_fix = FAILED_GPS;
			GPS_timer = DIYmillis();
			Serial.println("XXX \t No GPS, last 10s \t ***");
		}
	}
}
	
 /****************************************************************
 * 
 ****************************************************************/
void IMU_join_data()
{
	imu_messages_received++;
	int j=0;

	 //Storing IMU roll
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	roll_sensor = intUnion.word;

	 //Storing IMU pitch
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	pitch_sensor = intUnion.word;

	 //Storing IMU heading (yaw)
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	ground_course = intUnion.word;

	GPS_update |= GPS_IMU;
}

 /****************************************************************
 * 
 ****************************************************************/
void GPS_join_data()
{
	gps_messages_received++;
	int j=0;				 

	current_loc.lng = join_4_bytes(&IMU_buffer[j]);		// Lat and Lon * 10**7
	j += 4;

	current_loc.lat = join_4_bytes(&IMU_buffer[j]);
	j += 4;

	//Storing GPS Height above the sea level
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	current_loc.alt = (long)intUnion.word * 10;		 //	Altitude in meters * 100 

	//Storing Speed (3-D) 
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	ground_speed = (float)intUnion.word;			// Speed in M/S * 100
	
	//We skip the gps ground course because we use yaw value from the IMU for ground course
	j += 2;
	iTOW = join_4_bytes(&IMU_buffer[j]);		//	Time of Week in milliseconds
        j +=4;
        imu_health = IMU_buffer[j++];
	
	GPS_update |= GPS_BOTH;
	GPS_fix = VALID_GPS;
	print_telemetry = true;
}
				
 /****************************************************************
 * 
 ****************************************************************/
void PERF_join_data()
{ 
	int j=0;				 
	perf_mon_timer = join_4_bytes(&IMU_buffer[j]);		// time in milliseconds of reporting interval
	j += 4;
	
	//IMU main loop cycles in reporting interval
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	IMU_mainLoop_count = intUnion.word ;
	
	//Max IMU main loop time in milliseconds 
	intUnion.byte[0] = IMU_buffer[j++];
	intUnion.byte[1] = IMU_buffer[j++];
	G_Dt_max = (float)intUnion.word;			
	
	gyro_sat_count = IMU_buffer[j++];
	adc_constraints = IMU_buffer[j++];
	renorm_sqrt_count = IMU_buffer[j++];
	renorm_blowup_count = IMU_buffer[j++];
	gps_payload_error_count = IMU_buffer[j++];
	gps_checksum_error_count = IMU_buffer[j++];
	gps_pos_fix_count = IMU_buffer[j++];
	gps_nav_fix_count = IMU_buffer[j++];
	gps_messages_sent = IMU_buffer[j++];
}
						

void checksum(byte data)
{
	ck_a+=data;
	ck_b+=ck_a; 
}

void wait_for_data(byte many)
{
	while(Serial.available() <= many); 
}

 // Join 4 bytes into a long
 // -------------------------
int32_t join_4_bytes(byte Buffer[])
{
	longUnion.byte[0] = *Buffer;
	longUnion.byte[1] = *(Buffer+1);
	longUnion.byte[2] = *(Buffer+2);
	longUnion.byte[3] = *(Buffer+3);
	return(longUnion.dword);
}


#endif



