#ifndef VALUE_MODIFIERS_HEADER
#define VALUE_MODIFIERS_HEADER

void pulse_float(float &value, bool &value_state, float speed, float min_value, float max_value);


//#define VALUE_MODIFIERS_IMPLEMENTATION
#ifdef VALUE_MODIFIERS_IMPLEMENTATION

void pulse_float(float &value, bool &value_state, float speed, float min_value, float max_value)
{
	if (value_state)
	{
		value += speed;
		if (value >= max_value)
		{
			value = max_value;
			value_state = false;
		}
	}
	else
	{
		value -= speed;
		if (value <= min_value)
		{
			value = min_value;
			value_state = true;
		}
	}

}


#endif

#endif