// Reads packet from GCS into buffer via XBee
// Returns size of received packet in bytes

// TODO: set define for XBEE_BAUD_RATE
// TODO: set define for XBEE_RTS_PIN
// TODO: set define for MAX_GCS_PACKET_SIZE
// TODO: set define for GPS_BAUD_RATE

#define XBEE_RTS_PIN 50
#define XBEE_RTS_PIN2 51
#define XBEE_RTS_PIN3 52
#define MAX_GCS_PACKET_SIZE 16

struct Location {
  long lat;
  long lng;
  long alt;
};

struct GCS_packet_t {
  struct Location next_WP;  // 12 bytes
  long checksum;	          // 4 bytes
};

int xbee_read (struct GCS_packet_t *buf)
{
        
	int i = 0;
        byte *ptr = (byte*) buf;
	// tell XBee to release whatever packet is in its transmit buffer
        Serial1.flush();
	digitalWrite(XBEE_RTS_PIN, LOW);
        //while(Serial1.available() == 0);
        delay(0);  
        delay(0);       
        while (Serial1.available() > 0) // i < MAX_GCS_PACKET_SIZE && 
            ptr[i++] = (byte)Serial1.read(); 
                  
	Serial1.flush();

	//implement a checksum function to ensure data integrity
	return	(i == 0) ? 0 : 
		(i == MAX_GCS_PACKET_SIZE && buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
}

int xbee_read2 (struct GCS_packet_t *buf)
{
        
	int i = 0;
        byte *ptr = (byte*) buf;
	// tell XBee to release whatever packet is in its transmit buffer
        Serial2.flush();
	digitalWrite(XBEE_RTS_PIN2, LOW);
        //while(Serial1.available() == 0);
        delay(0);  
        delay(0);       
        while (Serial2.available() > 0) // i < MAX_GCS_PACKET_SIZE && 
            ptr[i++] = (byte)Serial2.read(); 
                  
	Serial2.flush();

	//implement a checksum function to ensure data integrity
	return	(i == 0) ? 0 : 
		(i == MAX_GCS_PACKET_SIZE && buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
} 
int xbee_read3 (struct GCS_packet_t *buf)
{
        
	int i = 0;
        byte *ptr = (byte*) buf;
	// tell XBee to release whatever packet is in its transmit buffer
        Serial3.flush();
	digitalWrite(XBEE_RTS_PIN3, LOW);
        //while(Serial1.available() == 0);
        delay(0);  
        delay(0);       
        while (Serial3.available() > 0) // i < MAX_GCS_PACKET_SIZE && 
            ptr[i++] = (byte)Serial3.read(); 
                  
	Serial3.flush();

	//implement a checksum function to ensure data integrity
	return	(i == 0) ? 0 : 
		(i == MAX_GCS_PACKET_SIZE && buf->checksum == (buf->next_WP.lat + buf->next_WP.alt)) ? i : -1;
} 

void setup()
{
   pinMode(XBEE_RTS_PIN, OUTPUT);
   pinMode(XBEE_RTS_PIN2, OUTPUT);
   pinMode(XBEE_RTS_PIN3, OUTPUT);
   Serial.begin(115200);
   Serial1.begin(115200);
   Serial2.begin(115200);
   Serial3.begin(115200);
}
void loop()
{
  
   digitalWrite(XBEE_RTS_PIN, HIGH);
   digitalWrite(XBEE_RTS_PIN2, HIGH);
   digitalWrite(XBEE_RTS_PIN3, HIGH);
      union{long dword;
          byte bytes[4];
          }; 
    dword = 123456;
   Serial1.write(bytes,4);
    dword =785;
   Serial1.write(bytes,4);
    dword = 98;
   Serial1.write(bytes,4);
  delay(1000);
  dword = 99999;
    Serial2.write(bytes,4);
    delay(1000);
    dword = 31337;
    Serial3.write(bytes,4);
   Serial.println("Sent both...");
   delay(1000);
   if(Serial1.available() > 1)
     Serial1.println("Data got into 1 buffer");
   if(Serial2.available() > 1)
     Serial2.println("Data got into 2 buffer"); 
  GCS_packet_t packet, packet2, packet3;
  packet.next_WP.lat = 0;
  packet.next_WP.lng = 0;
  packet.next_WP.alt = 0;
  packet.checksum = 0;
  
  packet2.next_WP.lat = 0;
  packet2.next_WP.lng = 0;
  packet2.next_WP.alt = 0;
  packet2.checksum = 0; 
  
  packet3.next_WP.lat = 0;
  packet3.next_WP.lng = 0;
  packet3.next_WP.alt = 0;
  packet3.checksum = 0;
  
  int size = xbee_read(&packet);
  int size2 = xbee_read2(&packet2);
  int size3 = xbee_read3(&packet3);
  Serial.println(size);
  Serial.println(packet.next_WP.lat);
  Serial.println(packet.next_WP.lng);
  Serial.println(packet.next_WP.alt);
  Serial.println(packet.checksum);
  Serial.println();
  Serial.println(size2);
  Serial.println(packet2.next_WP.lat);
  Serial.println(packet2.next_WP.lng);
  Serial.println(packet2.next_WP.alt);
  Serial.println(packet2.checksum);
  Serial.println();
  Serial.println(size3);
  Serial.println(packet3.next_WP.lat);
  Serial.println(packet3.next_WP.lng);
  Serial.println(packet3.next_WP.alt);
  Serial.println(packet3.checksum);
  Serial.println();
  
}

