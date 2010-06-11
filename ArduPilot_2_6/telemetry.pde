// Reads packet from GCS into buffer via XBee
// Returns size of received packet in bytes

int xbee_read (struct GCS_packet_t *buf)
{
	int i = 0;
	int temp;
	byte *ptr = (byte *) buf;

	/*
	// reset Serial to XBee configuration
	Serial.end();
	Serial.begin(XBEE_BAUD_RATE);
	*/
	//Serial.flush();

	// tell XBee to release whatever packet is in its transmit buffer
	//digitalWrite(XBEE_RTS_PIN, LOW);
							// DELAY REFERENCE W/ BAUD RATES
	//delay(0);		// Baud Rate: 9600  delay(20)
	//delay(0);		// Baud Rate: 57600 delay(3)
					    // Baud Rate: 115200 use two delay(0) for best reliability
	//delay(3);

	// read packet into buffer
	while (i < GCS_MAX_PACKET_SIZE && xbeeSerial.available()){
		ptr[i++] = (byte)xbeeSerial.read();
		delay(2);
	}

	// make sure XBee holds onto packets until the function is called again
	//digitalWrite(XBEE_RTS_PIN, HIGH);

	// set Serial config back to GPS's needs
	//Serial.flush();
	/*
	Serial.end();
	Serial.begin(GPS_BAUD_RATE);
	*/
	if (i == GCS_MAX_PACKET_SIZE) {
		Serial.print(buf->next_WP.lat, DEC);
		Serial.print(" ");
		Serial.println(buf->next_WP.lng, DEC);
		Serial.print(" ");
		Serial.println(buf->next_WP.alt, DEC);
		Serial.print(" ");
		Serial.println(buf->checksum, DEC);
	}
	else if (i > 0)
		Serial.println("Didn't finish getting a packet");

	//implement a checksum function to ensure data integrity
	return 	(i == 0 || i < GCS_MAX_PACKET_SIZE) ? 0 :
					(buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
}
