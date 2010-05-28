// Reads packet from GCS into buffer via XBee
// Returns size of received packet in bytes

// TODO: set define for XBEE_BAUD_RATE
// TODO: set define for XBEE_RTS_PIN
// TODO: set define for MAX_GCS_PACKET_SIZE
// TODO: set define for GPS_BAUD_RATE

int xbee_read (struct GCS_packet_t *buf)
{
	int i = 0;

	//digitalWrite(GPS_RTS_PIN, HIGH);

	// reset Serial to XBee configuration
	Serial.flush();	// XXX: This could be problematic
//	Serial.end();
//	Serial.begin(XBEE_BAUD_RATE);

	// tell XBee to release whatever packet is in its transmit buffer
	//digitalWrite(XBEE_RTS_PIN, LOW);

	// read packet into buffer
	byte *ptr = (byte *) buf;
	while (i < MAX_GCS_PACKET_SIZE && Serial.available() > 0)
		ptr[i++] = (byte)Serial.read();

	// make sure XBee holds onto packets until the function is called again
	//digitalWrite(XBEE_RTS_PIN, HIGH);

	// set Serial config back to GPS's needs
//	Serial.end();
//	Serial.begin(GPS_BAUD_RATE);
	Serial.flush();

	//digitalWrite(GPS_RTS_PIN, LOW);

	//implement a checksum function to ensure data integrity
	return	(i == 0) ? 0 : 
		(i == MAX_GCS_PACKET_SIZE && buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
}
