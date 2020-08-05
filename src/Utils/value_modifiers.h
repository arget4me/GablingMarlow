#ifndef VALUE_MODIFIERS_HEADER
#define VALUE_MODIFIERS_HEADER

void pulse_float(float &value, bool &value_state, float speed, float min_value, float max_value);
void loop_float(float &value, float speed, float min_value, float max_value);


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

void loop_float(float& value, float speed, float min_value, float max_value)
{
	value += speed;
	if (value > max_value)
	{
		int x = (value - min_value) / (max_value - min_value);

		value = value - x * (max_value - min_value);
	}

	if (value < min_value)
	{
		int x = (value - min_value) / (max_value - min_value);

		value = max_value + value - x * (max_value - min_value);
	}
}

#endif

#endif