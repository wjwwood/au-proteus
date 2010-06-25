
byte switchPosition 	= 0;
byte oldSwitchPosition 	= 0;

void read_control_switch()
{
	byte switchPosition = readSwitch();
	if (oldSwitchPosition != switchPosition){

		// tell user about it, it is up to them to deal with it!
		switch_event(switchPosition);
		oldSwitchPosition = switchPosition;

		// reset navigation integrators
		// -------------------------
		reset_I();
	}
}

void reset_control_switch()
{
	oldSwitchPosition = 0;
	read_control_switch();
}

byte readSwitch(void){
	if(digitalRead(4) == HIGH){
		if(digitalRead(5) == HIGH){

			// Middle Switch Position
			// ----------------------
			return 2;

		}else{
			// 3rd Switch Position
			// -------------------
			return 3;
		}
	}else{
		// 1st Switch Position
		// ----------------------
		return 1;
	}
}

void initControlSwitch()
{
	oldSwitchPosition = switchPosition = readSwitch();
}
