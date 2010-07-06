/*****************************************
 * Proportional Integrator Derivative Control 
 *****************************************/

float PID(long PID_error, long dt, int PID_case)
{

	// PID_case is used to keep track of which PID controller is being used - e.g.  PID_servo_roll
	float output;
 
	float derivative = 1000.0f * (PID_error-Last_error[PID_case]) / (float)dt;
        Last_error[PID_case] = PID_error;

	output = (kp[PID_case]*PID_error);    	//Compute proportional component
											//Positive error produces positive output

	Integrator[PID_case] += (float)PID_error*(float)dt/1000.0f; 
	Integrator[PID_case] = constrain(Integrator[PID_case],-Integrator_max[PID_case],Integrator_max[PID_case]);
  
	//Integrator[PID_case] = reset_I(Integrator[PID_case]);   
  
	output += Integrator[PID_case]*ki[PID_case];        	//Add the integral component

	output += derivative*kd[PID_case];                   	//Add the derivative component

	return output; //Returns the result     

}

// Zeros out navigation Integrators if we are changing mode, have passed a waypoint, etc.
// Keeps outdated data out of our calculations
void reset_I(void)
{
		Integrator[CASE_NAV_ROLL] = 0;
		Integrator[CASE_NAV_PITCH] = 0;
		Integrator[CASE_TE_THROTTLE] = 0;
		Integrator[CASE_FBW_THROTTLE] = 0;
		Last_error[CASE_NAV_ROLL] = 0;
		Last_error[CASE_NAV_PITCH] = 0;
		Last_error[CASE_TE_THROTTLE] = 0;
		Last_error[CASE_FBW_THROTTLE] = 0;
}

