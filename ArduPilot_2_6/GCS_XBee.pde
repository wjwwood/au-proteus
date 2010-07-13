// GCS code for XBee GCS
// Also uses print_control_mode() function from GCS_ArduPilot_Standard

#if GCS_PROTOCOL == 6
// a print position function more amiable for Java XBee GCS
void print_position(void)
{
			Serial.flush();

			longUnion.dword = current_loc.lat/10;
			Serial.write(longUnion.byte,4);

			longUnion.dword = current_loc.lng/10;
			Serial.write(longUnion.byte,4);

			longUnion.dword = current_loc.alt/100;
			Serial.write(longUnion.byte,4);

			longUnion.dword = next_WP.lat/10;
			Serial.write(longUnion.byte,4);

			longUnion.dword = next_WP.lng/10;
			Serial.write(longUnion.byte,4);

			longUnion.dword = next_WP.alt/100;
			Serial.write(longUnion.byte,4);

			longUnion.dword = ground_speed/100;
			Serial.write(longUnion.byte,4);		

			longUnion.dword = target_bearing/100;
			Serial.write(longUnion.byte,4);

#ifdef XBEE_READ
			longUnion.dword = (fakeWP) ? 997 : wp_index;
#else
			longUnion.dword = wp_index;
#endif
			Serial.write(longUnion.byte,4);//Actually is the waypoint.

			longUnion.dword = wp_distance;
			Serial.write(longUnion.byte,4);

			Serial.flush();
}

// dummy print_current_waypoint()
void print_current_waypoints(){ }

// dummy print_attitude()
void print_attitude() { }

#endif
