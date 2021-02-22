#ifndef STRUCTURED_BINARY_WRAPPER_HEADER
#define STRUCTURED_BINARY_WRAPPER_HEADER

#include "structured_binary_io.h"


namespace STRUCTURED_IO
{

	StructuredData* create_new_structured_data(const char* name);

	StructuredDataValue* add_int_value(StructuredDataValue** value_location, int value);
	StructuredDataValue* add_float_value(StructuredDataValue** value_location, float value);
	StructuredDataValue* add_text_value(const char* text, int length);
	StructuredDataValue* add_text_null_terminated_value(const char* text);
	StructuredDataValue* add_list_value(int list_size);
	bool add_value_to_list(StructuredDataList* list, StructuredDataValue* value_heap_allocated, int index);
	bool add_value_to_end_list(StructuredDataList* list, StructuredDataValue* value_heap_allocated);

	StructuredDataList* get_list_from_structure(StructuredDataValue* value);
	float* get_float_from_structure(StructuredDataValue* value);
	int* get_int_from_structure(StructuredDataValue* value);
	char* get_text_null_terminated_from_structure(StructuredDataValue* value);

	bool check_list_contains_string(StructuredDataList* list, char* null_terminated_string, int string_buffer_length = 0);
}

//#define STRUCTURED_BINARY_IO_IMPLEMENTATION
#ifdef STRUCTURED_BINARY_IO_IMPLEMENTATION


namespace STRUCTURED_IO
{
	StructuredData* create_new_structured_data(const char* name)
	{
		StructuredData* data = new STRUCTURED_IO::StructuredData;
		data->name = new char[128]; 
		for (int i = 0; i < 128; i++)
		{
			char current_character = name[i];
			data->name[i] = current_character;

			if (current_character == '\0')
			{
				break;
			}
		}
		data->name[127] = '\0';

		return data;
	}


	StructuredDataValue* add_int_value(StructuredDataValue** value_location, int value)
	{
		StructuredDataValue* data = new StructuredDataValue;
		data->value_type = StructuredDataValueType::INT_TYPE;
		data->value = new int(value);

		return data;
	}

	StructuredDataValue* add_float_value(StructuredDataValue** value_location, float value)
	{
		StructuredDataValue* data = new StructuredDataValue;
		data->value_type = StructuredDataValueType::FLOAT_TYPE;
		data->value = new float(value);

		return data;
	}

	StructuredDataValue* add_text_value(const char* text, int length)
	{
		if (length > 0)
		{
			StructuredDataValue* data = new StructuredDataValue;
			data->value_type = StructuredDataValueType::STRING_TYPE;
			if (text[length - 1] == '\0')
			{
				data->value = new char[length];
			}
			else
			{
				//need to add an extra space for the terminal character
				int length_padded = length + 1;
				data->value = new char[length_padded];
				((char*)(data->value))[length] = '\0';
			}

			for (int i = 0; i < length; i++)
			{
				((char*)(data->value))[i] = text[i];
			}			

			return data;
		}
		else
		{
			return nullptr;
		}
	}

	StructuredDataValue* add_text_null_terminated_value(const char* text)
	{
		
		StructuredDataValue* data = new StructuredDataValue;
		data->value_type = StructuredDataValueType::STRING_TYPE;
		int length = 0;
		constexpr int MAX_TEXT_LENGTH = 1000;
		for (int i = 0; i < MAX_TEXT_LENGTH; i++)
		{
			if (text[i] == '\0')
			{
				i++;//include the ermination character
				length = i;
			}
		}

		data->value = new char[length];
		for (int i = 0; i < length; i++)
		{
			((char*)(data->value))[i] = text[i];
		}

		if(((char*)(data->value))[length - 1] != '\0')
		{
			((char*)(data->value))[length - 1] = '\0';
		}

		return data;
	}

	StructuredDataValue* add_list_value(int list_size)
	{
		if (list_size > 0)
		{
			StructuredDataValue* data = new StructuredDataValue;
			data->value_type = StructuredDataValueType::LIST_TYPE;
			StructuredDataList* list = new StructuredDataList;
			data->value = list;

			list->list_size = list_size;
			list->value = new StructuredDataValue*[list_size];

			return data;
		}
		else
		{
			return nullptr;
		}
	}

	bool add_value_to_list(StructuredDataList* list, StructuredDataValue* value_heap_allocated, int index)
	{
		if (list != nullptr && list->value != nullptr && value_heap_allocated != nullptr && index >= 0 && index < list->list_size)
		{
			*(list->value + index) = value_heap_allocated;

			return true;
		}
		else
		{
			return false;
		}
	}


	bool add_value_to_end_list(StructuredDataList* list, StructuredDataValue* value_heap_allocated)
	{
		if (list != nullptr && list->value != nullptr && value_heap_allocated != nullptr)
		{
			const int new_list_size = list->list_size + 1;
			StructuredDataValue** list_array_resized = new StructuredDataValue*[new_list_size];
			if (list_array_resized)
			{
				for (int i = 0; i < list->list_size; i++)
				{
					*(list_array_resized + i) = *(list->value + i);
				}

				delete[] list->value;
				list->value = list_array_resized;

				const int last_index = list->list_size;
				*(list->value + last_index) = value_heap_allocated;
				list->list_size = new_list_size;


				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}



	StructuredDataList* get_list_from_structure(StructuredDataValue* value)
	{
		if (value != nullptr && value->value_type == StructuredDataValueType::LIST_TYPE)
		{
			return (StructuredDataList*) value->value;
		}
		return nullptr;
	}

	float* get_float_from_structure(StructuredDataValue* value)
	{
		if (value != nullptr && value->value_type == StructuredDataValueType::FLOAT_TYPE)
		{
			return (float*)value->value;
		}
		return nullptr;
	}

	int* get_int_from_structure(StructuredDataValue* value)
	{
		if (value != nullptr && value->value_type == StructuredDataValueType::INT_TYPE)
		{
			return (int*)value->value;
		}
		return nullptr;
	}

	char* get_text_null_terminated_from_structure(StructuredDataValue* value)
	{
		if (value != nullptr && value->value_type == StructuredDataValueType::STRING_TYPE)
		{
			return (char*)value->value;
		}
		return nullptr;
	}

#define VALUE_COMPARE_IMPLEMENTATION
#include <Utils/value_compare.h>
#undef VALUE_COMPARE_IMPLEMENTATION

	bool check_list_contains_string(StructuredDataList* list, char* null_terminated_string, int string_buffer_length)
	{
		bool contains = false;
		if (list != nullptr && null_terminated_string != nullptr)
		{
			int max_buffer_length = 128;
			if (string_buffer_length >= 0)
			{
				max_buffer_length = string_buffer_length;
			}
			
			for (int i = 0; i < list->list_size; i++)
			{
				char* compare_string = get_text_null_terminated_from_structure(list->value[i]);
				if (compare_string != nullptr)
				{
					if (VALUE_UTILS::null_terminated_char_string_equals(null_terminated_string, compare_string, max_buffer_length))
					{
						contains = true;
						break;
					}
				}
			}
		}
	
		return contains;
	}

}



#endif
#endif