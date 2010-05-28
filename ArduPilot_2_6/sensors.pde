#if GPS_PROTOCOL != 5
void read_analogs()
{
	#if GPS_PROTOCOL != 3
		analog0 = analogRead(0);
		analog1 = analogRead(1);
		roll_sensor  = getRoll() + ROLL_TRIM;
		pitch_sensor = getPitch() + PITCH_TRIM;
	
		#if ENABLE_Z_SENSOR == 0
			if (analog0 > 511){
				ir_max = max((abs(511 - analog0) * IR_MAX_FIX), ir_max);
				ir_max = constrain(ir_max, 40, 600);
			}
		#endif	
		#if ENABLE_Z_SENSOR == 1	
			//Checks if the roll is less than 10 degrees to read z sensor
			if(abs(roll_sensor) <= 1000){
				analog2 = ((float)analogRead(2) * 0.10) + ((float)analog2 * .90);
				ir_max = abs(511 - analog2) * IR_MAX_FIX;
				ir_max = constrain(ir_max, 40, 600);
			}
		#endif
	#endif
	
	// Read Airspeed
	#if AIRSPEED_SENSOR == 1
		analog3 = ((float)analogRead(3) * .10) + (analog3 * .90);
		airspeed_current = (int)analog3 - airspeed_offset;
	#endif

	#if BATTERY_EVENT == 1
		analog5 = ((float)analogRead(5)*.01) + ((float)analog5*.99);
		battery_voltage = BATTERY_VOLTAGE(analog5);
		if(battery_voltage < INPUT_VOLTAGE)
			low_battery_event();
	#endif
	
}
#endif

#if GPS_PROTOCOL == 5
void read_analogs()
{
	
	roll_sensor = ((float)roll_sensor *.8f) + ((float)nav_roll *.2f);
	pitch_sensor = ((float)pitch_sensor *.8f) + ((float)nav_pitch *.2f);
	
#if THROTTLE_OUT == 0
	servo_throttle = throttle_cruise;
#endif
	airspeed_current = throttle_cruise + ((float)airspeed_current *.9f) + ((float)servo_throttle *.1f);
	airspeed_current = constrain(airspeed_current, 0, THROTTLE_MAX);
}
#endif




// returns the sensor values as degrees of roll
//   0 ----- 511  ---- 1023    IR Sensor
// -90°       0         90°	    degree output * 100
// sensors are limited to +- 60° (6000 when you multply by 100)

long getRoll(void)
{
	#if XY_SENSOR_LOCATION ==1
	return constrain((x_axis() + y_axis()) / 2, -6000, 6000);
	#endif
	
	#if XY_SENSOR_LOCATION ==0
	return constrain((-x_axis() - y_axis()) / 2, -6000, 6000);
	#endif

	#if XY_SENSOR_LOCATION ==3
	return constrain((-x_axis() - y_axis()) / 2, -6000, 6000);
	#endif
	
	#if XY_SENSOR_LOCATION ==2
	return constrain((x_axis() + y_axis()) / 2, -6000, 6000);
	#endif
}

long getPitch(void)
{
  #if XY_SENSOR_LOCATION ==1
  return constrain((-x_axis() + y_axis()) / 2, -6000, 6000);
  #endif
  
  #if XY_SENSOR_LOCATION ==0
  return constrain((x_axis() - y_axis()) / 2, -6000, 6000);
  #endif

  #if XY_SENSOR_LOCATION ==3
  return constrain((-x_axis() + y_axis()) / 2, -6000, 6000);
  #endif
  
  #if XY_SENSOR_LOCATION ==2
  return constrain((x_axis() - y_axis()) / 2, -6000, 6000);
  #endif
}

long x_axis(void)// roll
{
	return ((analog1 - 511l) * 9000l) / ir_max;
	//      611 - 511 
	//         100 * 9000 / 100 = 90°  low = underestimate  = 36 looks like 90 = flat plane or bouncy plane
	//         100 * 9000 / 250 = 36°   				    = 36 looks like 36
	//		   100 * 9000 / 500 = 18°  high = over estimate = 36 looks like 18 = crash plane
}

long y_axis(void)// pitch
{
	return ((analog0 - 511l) * 9000l) / ir_max;
}


void zero_airspeed(void)
{
	analog3 = analogRead(3);
	for(int c=0; c < 80; c++){
		analog3 = (analog3 * .90) + ((float)analogRead(3) * .10);	
	}
	airspeed_offset = analog3;
	
}



/*

IR sesor looks at the difference of the two readings and gives a value the same = 511
differnce is +=40°
(temp dif / 40) * 511
if the top sees the ground its the dif is positive or negative


Analog 0 = Pitch sensor
Analog 0 = Roll sensor - unmarked
   	
	  					   ^ GROUND
		285			 	 713
					 	 P
			 \			/
			  \		  /
				\	/
				511
				/	\
			  /		 \
			/		  \
		 P
	  300				707
	 			||||
				||||
				||||
			 cable			 
				
				
 */



