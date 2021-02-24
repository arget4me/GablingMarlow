#ifndef VALUE_COMPARE_HEADER
#define VALUE_COMPARE_HEADER


namespace VALUE_UTILS {
int null_terminated_char_string_length_including_termination(char* string_one, int max_character_check);
int null_terminated_char_string_length(char* string_one, int max_character_check);
bool null_terminated_char_string_equals(char* string_one, char* string_two, int max_character_check, bool ignore_caps = false);
}




#define VALUE_COMPARE_IMPLEMENTATION
#ifdef VALUE_COMPARE_IMPLEMENTATION

namespace VALUE_UTILS{
int null_terminated_char_string_length_including_termination(char* string_one, int max_character_check)
{
	int length = null_terminated_char_string_length(string_one, max_character_check) + 1;
	return length;
}


int null_terminated_char_string_length(char* string_one, int max_character_check)
{
	int length = 0;
	if (string_one != nullptr && max_character_check < 1000)
	{
		for (int i = 0; i < max_character_check; i++)
		{
			if (string_one[i] == '\0')
			{
				break;
			}
			length++;
		}
	}

	return length;
}

bool null_terminated_char_string_equals(char* string_one, char* string_two, int max_character_check, bool ignore_caps)
{
	//@Note: This assumes strings are ascii formated. or at least that the first 128 characters are ascii formated.

	bool strings_are_equal = true;
	if (string_one != nullptr && string_two != nullptr && max_character_check < 1000)
	{
		for (int i = 0; i < max_character_check; i++)
		{
			char& char_one = string_one[i];
			char& char_two = string_two[i];

			bool one_is_alphabetical = ((char_one >= 'A' && char_one <= 'Z')) || ((char_one >= 'a' && char_one <= 'z'));
			bool two_is_alphabetical = ((char_two >= 'A' && char_two <= 'Z')) || ((char_two >= 'a' && char_two <= 'z'));

			if ((ignore_caps == false) && (one_is_alphabetical && two_is_alphabetical))
			{
				int caps_diff = 'a' - 'A';
				int words_diff = char_one - char_two;
				if (words_diff < 0)
				{
					words_diff = char_two - char_one;
				}


				if (!((words_diff == caps_diff) || (words_diff == 0)))
				{
					return false;
				}
				else if (char_one == '\0' || char_two == '\0') //@Note: to the preceding if statement allows this to be more compactly written, chosing not to for code clearity.
				{
					strings_are_equal = true;
					break;
				}
			}
			else
			{
				if (char_one != char_two)
				{
					strings_are_equal = false; //Initialized to false, but this is more clear.
					break;
				}
				else if (char_one == '\0' || char_two == '\0') //@Note: to the preceding if statement allows this to be more compactly written, chosing not to for code clearity.
				{
					strings_are_equal = true;
					break;
				}
			}			
		}
	}
	else
	{
		strings_are_equal = false;
	}

	return strings_are_equal;
}
}

#endif
#endif