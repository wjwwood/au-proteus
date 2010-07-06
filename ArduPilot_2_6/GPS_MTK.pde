#if GPS_PROTOCOL == 4

#define BUF_LEN 60
byte MTK_buffer[BUF_LEN];

//MediaTek Checksum
byte ck_a 					= 0;
byte ck_b 					= 0;
byte MTK_class				= 0;
byte MTK_id					= 0;
byte MTK_payload_length_hi	= 26;
byte MTK_payload_counter	= 0;
byte MTK_ck_a 				= 0;
byte MTK_ck_b				= 0;

/****************************************************************
 * Here you have all the parsing stuff for MediaTek
 ****************************************************************/


void init_gps(void)
{
	Serial.begin(THIRTY_EIGHT_K_BAUD); //Universal Sincronus Asyncronus Receiveing Transmiting 
	
	GPS_update 	= GPS_NONE;
	GPS_fix 	= BAD_GPS;
}

void decode_gps(void)
{
	static unsigned long GPS_timer = 0;
	static byte GPS_step = 0;
	byte data;
	int numc;
	
	numc = Serial.available();
	if (numc > 0){
		for (int i=0; i < numc; i++){	// Process bytes received
			data = Serial.read();
			switch(GPS_step){		 //Normally we start from zero. This is a state machine
				case 0:	
					if(data==0xB5)	// MTK sync char 1
						GPS_step++;	 //ooh! first data packet is correct. Jump to the next step.
					break; 
				case 1:	
					if(data==0x62){	// MTK sync char 2
						GPS_step++;	 //ooh! The second data packet is correct, jump to the step 2
					} else {
						GPS_step=0;	 //Nope, incorrect. Restart to step zero and try again.
					}
					break;
				case 2:
					MTK_class = data;
					checksum(MTK_class);
					GPS_step++;
					break;
				case 3:
					MTK_id = data;
					GPS_step++;
					MTK_payload_counter = 0;
					checksum(MTK_id);
					break;

				case 4:
					if (MTK_payload_counter < MTK_payload_length_hi){  // We stay in this state until we reach the payload_length
						MTK_buffer[MTK_payload_counter] = data;
						checksum(data);
						MTK_payload_counter++;
						if (MTK_payload_counter == MTK_payload_length_hi){
							GPS_step++;
						}
					}
					break;

				case 5:
					MTK_ck_a = data;   // First checksum byte
					GPS_step++;
					break;

				case 6: // Payload data read...
					MTK_ck_b = data;   // Second checksum byte
					
					// We end the GPS read...
					if((ck_a == MTK_ck_a) && (ck_b == MTK_ck_b)){   	// Verify the received checksum with the generated checksum.. 
						//Serial.println(" ");
						GPS_join_data();               					// Parse the new GPS packet
						GPS_timer = millis(); //Restarting timer...
					}else{
						Serial.println("failed Checksum");
						Serial.flush();
						GPS_step = 0;
					}
					// Variable initialization
					GPS_step = 0;
					ck_a = 0;
					ck_b = 0;
					break;
			}
		}
	}
	
	if((DIYmillis() - GPS_timer) > 2000){
		if(GPS_fix != FAILED_GPS){
			GPS_fix = BAD_GPS;
		}
		GPS_update = GPS_NONE;

		if((DIYmillis() - GPS_timer) > 20000){
			invalid_location = true;
			GPS_fix = FAILED_GPS;
			GPS_timer = DIYmillis();
			Serial.println("no GPS, last 20s");
			//reinit the GPS Modules
		}
	}
}

/****************************************************************
 * 
 ****************************************************************/
void GPS_join_data()
{
	int j; // our Byte Offset
	//Verifing if we are in class 1, you can change this "IF" for a "Switch" in case you want to use other MTK classes.. 
	//In this case all the message im using are in class 1, to know more about classes check PAGE 60 of DataSheet.
	if(MTK_class == 0x01){
		switch(MTK_id){		
		    case 0x05: //ID Custom
				current_loc.lat 		= join_4_bytes(&MTK_buffer[0])*10;
				current_loc.lng 		= join_4_bytes(&MTK_buffer[4])*10;
				current_loc.alt 		= join_4_bytes(&MTK_buffer[8]); //alt_MSL M*100
				ground_speed 			= join_4_bytes(&MTK_buffer[12]); //M*100		
				ground_course	 		= join_4_bytes(&MTK_buffer[16]) / 10000; // Heading 2D
				NumSats					= MTK_buffer[20];
				GPS_fix					= MTK_buffer[21];
				iTOW 					= join_4_bytes(&MTK_buffer[22]);

				GPS_update = GPS_BOTH;

				if(GPS_fix > 0x01){
					GPS_fix = VALID_GPS;
					print_telemetry = true;
				}else{
					GPS_fix = BAD_GPS;
					print_telemetry = false;
				}
			break;
		}
	}
}

 // Join 4 bytes into a long
 // -------------------------
int32_t join_4_bytes(byte Buffer[])
{
	longUnion.byte[3] = *Buffer;
	longUnion.byte[2] = *(Buffer+1);
	longUnion.byte[1] = *(Buffer+2);
	longUnion.byte[0] = *(Buffer+3);
	return(longUnion.dword);
}

void checksum(byte data)
{
	//Serial.print(data, HEX);
	//Serial.print(",");
	ck_a += data;
	ck_b += ck_a; 
}

#endif


