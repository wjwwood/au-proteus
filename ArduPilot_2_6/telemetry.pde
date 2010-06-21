// Setup for XBee TX
void init_xbee()
{
  pinMode(XBEE_TX_PIN, INPUT); 
	// Assuming XBee TX pin is analog 4
	PCMSK1 = _BV(PCINT12);
	PCICR |= _BV(PCIE1);
	xbeeSerial.begin(XBEE_BAUD_RATE);
}

// XBee TX Interrupt Handler
ISR(PCINT1_vect)
{
	xbeeSerial.recv();
}

// Reads packet from GCS into buffer via XBee
// Returns size of received packet in bytes

int xbee_read (struct GCS_packet_t *buf)
{
	int i = 0;
	byte *ptr = (byte *) buf;

	// read packet into buffer
	while (i < GCS_MAX_PACKET_SIZE && xbeeSerial.available()){
		ptr[i++] = (byte)xbeeSerial.read();
	}

#ifdef XBEE_DEBUG
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
	else
		Serial.println("i = 0");
#endif
	
	xbeeSerial.flush();

	//implement a checksum function to ensure data integrity
	return 	(i == 0 || i < GCS_MAX_PACKET_SIZE) ? 0 :
					(buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
}
