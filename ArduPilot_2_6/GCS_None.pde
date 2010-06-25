#if GCS_PROTOCOL == -1
// A quieter GCS...

void print_current_waypoint()
{
}

void print_position(void)
{
}

void print_attitude(void)
{
}

// required by Groundstation to plot lateral tracking course 
void print_new_wp_info()
{
}

void print_control_mode(void)
{
	switch (control_mode){
		case MANUAL:
			Serial.println("###MANUAL\t0***");
			break;
		case STABILIZE:
			Serial.println("###STABILIZE\t1***");
			break;
		case CIRCLE:
			Serial.println("###CIRCLE\t1***");
			break;
		case FLY_BY_WIRE_A:
			Serial.println("###FLY BY WIRE A\t2***");
			break;
		case FLY_BY_WIRE_B:
			Serial.println("###FLY BY WIRE B\t2***");
			break;
		case AUTO:
			Serial.println("###AUTO\t5***");
			break;
		case RTL:
			Serial.println("###RTL\t6***");
			break;
		case LOITER:
			Serial.println("###LOITER\t7***");
			break;
	}
}

#endif