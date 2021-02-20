#ifndef STRUCTURED_BINARY_IMGUI_INTEGRATION_HEADER
#define STRUCTURED_BINARY_IMGUI_INTEGRATION_HEADER

#include <imgui.h>
#include "structured_binary_io.h"

void render_imgui_structured_binary(STRUCTURED_IO::StructuredData* data);

//#define STRUCTURED_BINARY_IMGUI_INTEGRATION_IMPLEMENTATION
#ifdef STRUCTURED_BINARY_IMGUI_INTEGRATION_IMPLEMENTATION

void render_imgui_structured_binary_recursive(STRUCTURED_IO::StructuredData* data);
void render_imgui_structured_value(STRUCTURED_IO::StructuredDataValue* value);
void render_imgui_structured_list(STRUCTURED_IO::StructuredDataList* list);

void render_imgui_structured_list(STRUCTURED_IO::StructuredDataList* list)
{
	if (list != nullptr)
	{
		for (int i = 0; i < list->list_size; i++)
		{
			STRUCTURED_IO::StructuredDataValue* current_value = list->value[i];
			render_imgui_structured_value(current_value);
		}
	}
}

void render_imgui_structured_value(STRUCTURED_IO::StructuredDataValue* value)
{
	if (value == nullptr)return;
	if (value->value_type == STRUCTURED_IO::StructuredDataValueType::INT_TYPE)
	{
		ImGui::Value("Int: ", *STRUCTURED_IO::get_int_from_structure(value));
	}
	else if (value->value_type == STRUCTURED_IO::StructuredDataValueType::FLOAT_TYPE)
	{
		ImGui::Value("Float: ", *STRUCTURED_IO::get_float_from_structure(value));
	}
	else if (value->value_type == STRUCTURED_IO::StructuredDataValueType::STRING_TYPE)
	{
		ImGui::Text(STRUCTURED_IO::get_text_null_terminated_from_structure(value));
	}
	else if (value->value_type == STRUCTURED_IO::StructuredDataValueType::LIST_TYPE)
	{
		render_imgui_structured_list(STRUCTURED_IO::get_list_from_structure(value));
	}
}

void render_imgui_structured_binary_recursive(STRUCTURED_IO::StructuredData* data)
{
	if (data != nullptr) {
	
		if (ImGui::TreeNode(data->name))
		{
			render_imgui_structured_value(data->value);
			ImGui::TreePop();
		}
		render_imgui_structured_binary_recursive(data->next);
	}
}

void render_imgui_structured_binary(STRUCTURED_IO::StructuredData* data)
{
	if (data != nullptr)
	{
		ImGui::Separator();
		render_imgui_structured_binary_recursive(data);
		ImGui::Separator();
	}
}

#endif

#endif