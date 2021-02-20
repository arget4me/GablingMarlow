#ifndef STRUCTURED_BINARY_IO_HEADER
#define STRUCTURED_BINARY_IO_HEADER

#include <cstdint>

/* GRAMMAR sort of: StructuredBinaryFileFormat (SBFF)
SBFF			-> [END] | TEXT VALUE SBFF
VALUE			-> NUMBER | TEXT | LIST
TEXT			-> [\TEXT] STRING
NUMBER			-> [\INT] [INT] | [\FLOAT] [FLOAT]
STRING			-> a:[INT] [CHAR][a]				//a:[INT] and [CHAR][a] MEANING there will be as many [CHAR] as the first [INT] says.
LIST			-> [\LIST] a:[INT] VALUE[a]		//a:[INT] and VALUE[a] MEANING there will be as many VALUE as the first INT says. 

------------------------------------------------------
Min token size is one byte.
------------------------------------------------------

[INT]			-> 4byte
[FLOAT]			-> 4byte
[CHAR]			-> 1byte

------------------------------------------------------

[END]		= 0xff
[\INT]		= 0x11
[\FLOAT]	= 0x22
[\TEXT]		= 0x33
[\LIST]		= 0x44

*/
namespace STRUCTURED_IO
{

enum class StructuredDataValueType
{
	INT_TYPE,
	FLOAT_TYPE,
	STRING_TYPE,
	LIST_TYPE,
};

typedef struct VALUE
{
	StructuredDataValueType value_type;
	void* value;
	VALUE();
}StructuredDataValue;

typedef struct LIST
{
	int list_size;
	StructuredDataValue** value;
	LIST();
}StructuredDataList;

typedef struct SBFF
{
	char* name;
	StructuredDataValue* value;
	struct SBFF* next;
	SBFF();
}StructuredData;

bool parse_structured_binary(int& token_index, char* buffer, int buffer_size, StructuredData** out_structured_data_allocate);
bool write_structured_binary(int& token_index, char* buffer, int buffer_size, StructuredData* data);
void get_size_bytes_structured_binary(int& num_bytes, StructuredData* data);
bool destroy_structured_data(StructuredData* data);

}

//#define STRUCTURED_BINARY_IO_IMPLEMENTATION
#ifdef STRUCTURED_BINARY_IO_IMPLEMENTATION

#define CONST_END ((char) 0xff)
#define CONST_INT ((char) 0x11)
#define CONST_FLOAT ((char) 0x22)
#define CONST_TEXT ((char) 0x33)
#define CONST_LIST ((char) 0x44)

namespace STRUCTURED_IO
{

//Constructors to set everything to nullptr. Ensures that the functions can check "if(POINTER == nullptr)"
VALUE::VALUE():value_type(StructuredDataValueType::INT_TYPE), value(nullptr) {};
LIST::LIST() :list_size(0), value(nullptr) {};
SBFF::SBFF() :name(nullptr), value(nullptr), next(nullptr) {};




bool get_size_bytes_string_null_terminated(int& num_bytes, char* string);
void get_size_bytes_structured_data_value(int& num_bytes, StructuredDataValue* value);
void get_size_bytes_structured_data_list(int& num_bytes, StructuredDataList* list);

bool write_char(int& token_index, char* buffer, int buffer_size, char outchar);
bool write_float(int& token_index, char* buffer, int buffer_size, float outfloat);
bool write_int(int& token_index, char* buffer, int buffer_size, int32_t outint);
bool write_string(int& token_index, char* buffer, int buffer_size, char* string, int string_length);
bool write_value(int& token_index, char* buffer, int buffer_size, StructuredDataValue* value);
bool write_list(int& token_index, char* buffer, int buffer_size, StructuredDataList* list);

bool parse_char(int& token_index, char* buffer, int buffer_size, char* outchar);
bool parse_float(int& token_index, char* buffer, int buffer_size, float& outfloat);
bool parse_int(int& token_index, char* buffer, int buffer_size, int32_t& outint);
bool parse_string(int& token_index, char* buffer, int buffer_size, char** outstring_allocate);
bool parse_value(int& token_index, char* buffer, int buffer_size, StructuredDataValue** out_datavalue_allocate);
bool parse_list(int& token_index, char* buffer, int buffer_size, StructuredDataList** out_list_allocate);

bool destroy_structured_data_value(StructuredDataValue* data);
bool destroy_structured_data_list(StructuredDataList* data);

//Write

bool write_string(int& token_index, char* buffer, int buffer_size, char* string, int string_length)
{
	if (token_index < buffer_size - (string_length * sizeof(char) - 1))
	{
		for (int i = 0; i < string_length; i++)
		{
			buffer[token_index++] = string[i];
		}

		return true;
	}

	return false;
}

bool write_char(int& token_index, char* buffer, int buffer_size, char outchar)
{
	if (token_index < buffer_size - (sizeof(char) - 1))
	{
		*(char*)(buffer + token_index) = outchar;
		token_index += sizeof(char);
		return true;
	}
	return false;
}

bool write_float(int& token_index, char* buffer, int buffer_size, float outfloat)
{
	if (token_index < buffer_size - (sizeof(float) - 1))
	{
		*(float*)(buffer + token_index) = outfloat;
		token_index += sizeof(float);
		return true;
	}
	return false;
}

bool write_int(int& token_index, char* buffer, int buffer_size, int32_t outint)
{
	if (token_index < buffer_size - (sizeof(int32_t) - 1))
	{
		*(int32_t*)(buffer + token_index) = outint;
		token_index += sizeof(int32_t);
		return true;
	}
	return false;
}

bool write_value(int& token_index, char* buffer, int buffer_size, StructuredDataValue* value)
{
	if (value != nullptr)
	{
		if (value->value_type == StructuredDataValueType::INT_TYPE)
		{
			if (write_char(token_index, buffer, buffer_size, CONST_INT))
			{
				if (write_int(token_index, buffer, buffer_size, *(int32_t*)value->value))
				{
					return true;
				}
			}
		}
		else if (value->value_type == StructuredDataValueType::FLOAT_TYPE)
		{
			if (write_char(token_index, buffer, buffer_size, CONST_FLOAT))
			{
				if (write_float(token_index, buffer, buffer_size, *(float*)value->value))
				{
					return true;
				}
			}
		}
		else if (value->value_type == StructuredDataValueType::STRING_TYPE)
		{
			int string_length = 0;
			if (get_size_bytes_string_null_terminated(string_length, (char*)value->value))
			{
				if (write_char(token_index, buffer, buffer_size, CONST_TEXT))
				{
					if (write_int(token_index, buffer, buffer_size, string_length))
					{
						if (write_string(token_index, buffer, buffer_size, (char*)value->value, string_length))
						{
							return true;
						}
					}
				}
			}
		}
		else if (value->value_type == StructuredDataValueType::LIST_TYPE)
		{
			if (write_char(token_index, buffer, buffer_size, CONST_LIST))
			{
				if (write_list(token_index, buffer, buffer_size, (StructuredDataList*)value->value))
				{
					return true;
				}
				
			}
		}
	}
	return false;	
}

bool write_list(int& token_index, char* buffer, int buffer_size, StructuredDataList* list)
{
	if (list != nullptr)
	{
		if (write_int(token_index, buffer, buffer_size, list->list_size))
		{
			for (int i = 0; i < list->list_size; i++)
			{
				StructuredDataValue* current_value = list->value[i];
				if (!write_value(token_index, buffer, buffer_size, current_value))
				{
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

bool write_structured_binary(int& token_index, char* buffer, int buffer_size, StructuredData* data)
{
	if (data == nullptr)
	{
		if (write_char(token_index, buffer, buffer_size, CONST_END))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if (token_index < buffer_size)
	{
		
		StructuredDataValue name_value;
		name_value.value = data->name;
		name_value.value_type = StructuredDataValueType::STRING_TYPE;

		if (write_value(token_index, buffer, buffer_size, &name_value))
		{
			if (write_value(token_index, buffer, buffer_size, data->value))
			{
				return write_structured_binary(token_index, buffer, buffer_size, data->next);
			}
		}
	}
	return false;
}


//Get size

bool get_size_bytes_string_null_terminated(int& num_bytes, char* string)
{
	if (string != nullptr)
	{
		int count = 0;
		int max_size = 1000;//Safetynet, don't get stuck in an infinite loop. But signal with return false if this terminates it
		while(string[count] != '\0')
		{
			count++;
			if (count > max_size)
			{
				return false;
			}
		}
		num_bytes += count;
	}
	return true;
}

void get_size_bytes_structured_data_value(int& num_bytes, StructuredDataValue* value)
{
	if (value != nullptr)
	{
		num_bytes++; //1 for the type identifier
		if (value->value_type == StructuredDataValueType::STRING_TYPE)
		{
			num_bytes += sizeof(int32_t); //4bytes For the length of string
			get_size_bytes_string_null_terminated(num_bytes, (char*)value->value);
		}
		else if (value->value_type == StructuredDataValueType::LIST_TYPE)
		{
			get_size_bytes_structured_data_list(num_bytes, (StructuredDataList*) value->value);
		}
		else
		{
			num_bytes += sizeof(int32_t); //4bytes
		}
	}
}

void get_size_bytes_structured_data_list(int& num_bytes, StructuredDataList* list)
{
	if (list != nullptr)
	{
		num_bytes+= sizeof(int32_t); //4bytes for the size of the list
		for (int i = 0; i < list->list_size; i++)
		{
			StructuredDataValue* current_value = list->value[i];
			get_size_bytes_structured_data_value(num_bytes, current_value);
		}
	}
}

void get_size_bytes_structured_binary(int& num_bytes, StructuredData* data)
	{
		if (data != nullptr)
		{
			{
				num_bytes++; //1 for the type identifier
				num_bytes += sizeof(int32_t); //4bytes For the length of string
				get_size_bytes_string_null_terminated(num_bytes, data->name);
			}
			get_size_bytes_structured_data_value(num_bytes, data->value);
			get_size_bytes_structured_binary(num_bytes, data->next);

			num_bytes++; //1 for the CONST_END character
		}
	}

//Parse

bool parse_structured_binary(int& token_index, char* buffer, int buffer_size, StructuredData** out_structured_data_allocate)
{
	if (token_index < buffer_size)
	{
		if (buffer[token_index] == CONST_END)
		{
			token_index++;
			return true;
		}
		StructuredData* data = new StructuredData;
		if (buffer[token_index] == CONST_TEXT)
		{
			token_index++;
			if (parse_string(token_index, buffer, buffer_size, &data->name))
			{
				if (parse_value(token_index, buffer, buffer_size, &data->value))
				{
					if (parse_structured_binary(token_index, buffer, buffer_size, &data->next))
					{
						*out_structured_data_allocate = data;
						return true;
					}
					else
					{
						return destroy_structured_data(data);
					}
				}
				else
				{
					return destroy_structured_data(data);
				}
			}
			else
			{
				return destroy_structured_data(data);
			}
		}
		else
		{
			return destroy_structured_data(data);
		}
	}
	return false;
}

bool parse_float(int& token_index, char* buffer, int buffer_size, float& outfloat)
{
	constexpr int FLOAT32_SIZE = sizeof(float);
	if (token_index < buffer_size - (FLOAT32_SIZE - 1)) //4bytes
	{
		outfloat = *(float*)(buffer + token_index); //Read 4 bytes as a float from char array
		token_index += FLOAT32_SIZE;
		return true;
	}
	return false;
}

bool parse_int(int& token_index, char* buffer, int buffer_size, int32_t& outint)
{
	constexpr int INT32_SIZE = sizeof(int32_t);
	if (token_index < buffer_size - (INT32_SIZE - 1)) //4bytes
	{
		outint = *(int32_t*)(buffer + token_index); //Read 4 bytes as an integer from char array
		token_index += INT32_SIZE;
		return true;
	}
	return false;
}

bool parse_char(int& token_index, char* buffer, int buffer_size, char* outchar)
{
	constexpr int CHAR_SIZE = sizeof(char);
	if (token_index < buffer_size - (CHAR_SIZE - 1)) //1byte
	{
		*outchar = *(buffer + token_index); //Read 1 bytes as a charcter from char array
		token_index += CHAR_SIZE;
		return true;
	}
	return false;
}

bool parse_string(int& token_index, char* buffer, int buffer_size, char** outstring_allocate)
{
	if (token_index < buffer_size)
	{
		int string_length = 0;
		if (parse_int(token_index, buffer, buffer_size, string_length))
		{
			const int padded_string_length = string_length + 1;// + 1 for termination character \0
			char* string = new char[padded_string_length];
			string[string_length] = '\0';
			for (int i = 0; i < string_length; i++)
			{
				char* current_character = string + i;
				if (!parse_char(token_index, buffer, buffer_size, current_character))
				{
					delete[] string;
					return false;
				}
			}

			*outstring_allocate = string;
			return true;
		}
	}

	return false;
}

bool parse_value(int& token_index, char* buffer, int buffer_size, StructuredDataValue** out_datavalue_allocate)
{
	if (token_index < buffer_size)
	{
		StructuredDataValue* data_value = new StructuredDataValue;
		if (buffer[token_index] == CONST_INT)
		{
			token_index++;
			data_value->value_type = StructuredDataValueType::INT_TYPE;
			data_value->value = new int;
			if (parse_int(token_index, buffer, buffer_size, *(int*)data_value->value))
			{
				*out_datavalue_allocate = data_value;
				return true;
			}
			else
			{
				delete data_value->value;
				delete data_value;

				return false;
			}
		}
		else if (buffer[token_index] == CONST_FLOAT)
		{
			token_index++;
			data_value->value_type = StructuredDataValueType::FLOAT_TYPE;
			data_value->value = new float;
			if (parse_float(token_index, buffer, buffer_size, *(float*)data_value->value))
			{
				*out_datavalue_allocate = data_value;
				return true;
			}
			else
			{
				delete data_value->value;
				delete data_value;
				
				return false;
			}
		}
		else if (buffer[token_index] == CONST_TEXT)
		{
			token_index++;
			data_value->value_type = StructuredDataValueType::STRING_TYPE;
			
			if (parse_string(token_index, buffer, buffer_size, (char**)&data_value->value))
			{
				*out_datavalue_allocate = data_value;
				return true;
			}
			else
			{
				delete data_value;
				return false;
			}
		}
		else if (buffer[token_index] == CONST_LIST)
		{
			token_index++;
			data_value->value_type = StructuredDataValueType::LIST_TYPE;

			if (parse_list(token_index, buffer, buffer_size, (StructuredDataList**)&data_value->value))
			{
				*out_datavalue_allocate = data_value;
				return true;
			}
			else
			{
				delete data_value;
				return false;
			}
		}
	}

	return false;
}

bool parse_list(int& token_index, char* buffer, int buffer_size, StructuredDataList ** out_list_allocate)
{
	if (token_index < buffer_size)
	{
		int list_size = 0;
		if (parse_int(token_index, buffer, buffer_size, list_size))
		{
			StructuredDataList* data_list = new StructuredDataList;
			data_list->list_size = list_size;
			data_list->value = new StructuredDataValue*[list_size];

			for (int i = 0; i < list_size; i++)
			{
				StructuredDataValue** current_value = data_list->value + i;
				if (!parse_value(token_index, buffer, buffer_size, current_value))
				{
					return destroy_structured_data_list(data_list);
				}
			}
			*out_list_allocate = data_list;
			return true;
		}
	}
	return false;
}

//Destroy

bool destroy_structured_data_value(StructuredDataValue* data)
{
	if (data != nullptr)
	{
		if (data->value_type == StructuredDataValueType::STRING_TYPE)
		{
			delete[] data->value;
			delete data;
		}
		else if (data->value_type == StructuredDataValueType::LIST_TYPE)
		{
			destroy_structured_data_list((StructuredDataList*)data->value);
			delete data;
		}
		else
		{
			delete data->value;
			delete data;
		}
		
	}

	return false;
}

bool destroy_structured_data_list(StructuredDataList* data)
{
	if (data != nullptr)
	{
		for (int i = 0; i < data->list_size; i++)
		{
			StructuredDataValue* current_value = data->value[i];
			destroy_structured_data_value(current_value);
		}
		delete[] data->value;
		delete data;
	}
	return false;
}

bool destroy_structured_data(StructuredData* data)
{
	if (data != nullptr)
	{
		delete[] data->name;

		destroy_structured_data_value(data->value);
		destroy_structured_data(data->next);
		delete data;
	}
	return false;
}


}

#endif

#endif


