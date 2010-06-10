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
	digitalWrite(XBEE_RTS_PIN, LOW);
							// DELAY REFERENCE W/ BAUD RATES
	//delay(0);		// Baud Rate: 9600  delay(20)
	//delay(0);		// Baud Rate: 57600 delay(3)
					    // Baud Rate: 115200 use two delay(0) for best reliability
	delay(3);

	// read packet into buffer
	while (i < GCS_MAX_PACKET_SIZE && (temp = xbeeSerial.read()) != -1)
		ptr[i++] = (byte)temp;

	// make sure XBee holds onto packets until the function is called again
	digitalWrite(XBEE_RTS_PIN, HIGH);

	// set Serial config back to GPS's needs
	//Serial.flush();
	/*
	Serial.end();
	Serial.begin(GPS_BAUD_RATE);
	*/

	//implement a checksum function to ensure data integrity
	return 	(i == 0 || i < GCS_MAX_PACKET_SIZE) ? 0 :
					(buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
}
