#ifndef STRUCTURED_BINARY_IMGUI_INTEGRATION_HEADER
#define STRUCTURED_BINARY_IMGUI_INTEGRATION_HEADER

#include <imgui.h>
#include "structured_binary_io.h"

void render_imgui_structured_binary();

//#define STRUCTURED_BINARY_IMGUI_INTEGRATION_IMPLEMENTATION
#ifdef STRUCTURED_BINARY_IMGUI_INTEGRATION_IMPLEMENTATION

void render_imgui_structured_binary_recursive(STRUCTURED_IO::StructuredData* data);
void render_imgui_structured_value(STRUCTURED_IO::StructuredDataValue* value);
void render_imgui_structured_list(STRUCTURED_IO::StructuredDataList* list);

void render_imgui_structured_list(STRUCTURED_IO::StructuredDataList* list)
{
	if (list == nullptr) return;
	ImGui::Separator();
	ImGui::Value("ListSize: ", list->list_size);
	for (int i = 0; i < list->list_size; i++)
	{
		STRUCTURED_IO::StructuredDataValue* current_value = list->value + 1;
		render_imgui_structured_value(current_value);
	}
	ImGui::Separator();
}

void render_imgui_structured_value(STRUCTURED_IO::StructuredDataValue* value)
{
	if (value == nullptr)return;
	if (value->value_type == STRUCTURED_IO::StructuredDataValueType::INT_TYPE)
	{
		ImGui::Value("Int: ", *(int*)value->value);
	}
	else if (value->value_type == STRUCTURED_IO::StructuredDataValueType::FLOAT_TYPE)
	{
		ImGui::Value("Float: ", *(float*)value->value);
	}
	else if (value->value_type == STRUCTURED_IO::StructuredDataValueType::STRING_TYPE)
	{
		ImGui::Text((char*)value->value);
	}
	else if (value->value_type == STRUCTURED_IO::StructuredDataValueType::LIST_TYPE)
	{
		render_imgui_structured_list((STRUCTURED_IO::StructuredDataList*) value->value);
	}
}

void render_imgui_structured_binary_recursive(STRUCTURED_IO::StructuredData* data)
{
	if (data != nullptr) {
	
		if (ImGui::TreeNode(data->name))
		{
			if (data->value == nullptr)
			{
				if (ImGui::Button("Add Int"))
				{
				}
				if (ImGui::Button("Add Float"))
				{
				}
				if (ImGui::Button("Add String"))
				{
				}
				if (ImGui::Button("Add List"))
				{
				}
			}
			else
			{
				render_imgui_structured_value(data->value);
			}
			ImGui::TreePop();
		}
		render_imgui_structured_binary_recursive(data->next);
		if (data->next == nullptr)
		{
			if (ImGui::Button("Add child"))
			{
				STRUCTURED_IO::StructuredData* new_data = new STRUCTURED_IO::StructuredData;
				data->next = new_data;
				const char* new_name = "Name";
				new_data->name = new char[128];
				new_data->name[127] = '\0';
				for (int i = 0; i < 4; i++)
				{
					new_data->name[i] = new_name[i];
				}
				new_data->name[5] = (char)rand();
				new_data->name[6] = (char)rand();
				new_data->name[7] = (char)rand();
				new_data->name[8] = (char)rand();
				new_data->name[9] = '\0';
			}
		}
	}
}

void render_imgui_structured_binary()
{

	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

	static STRUCTURED_IO::StructuredData* data;

	ImGui::Text("Structured Data Navigtor");
	

	if(data != nullptr)
		render_imgui_structured_binary_recursive(data);
	else
	{
		if (ImGui::Button("Add list"))
		{
			data = new STRUCTURED_IO::StructuredData;
			const char* new_name = "Name";
			data->name = new char[128];
			data->name[127] = '\0';
			for (int i = 0; i < 4; i++)
			{
				data->name[i] = new_name[i];
			}
			data->name[5] = (char)rand();
			data->name[6] = (char)rand();
			data->name[7] = (char)rand();
			data->name[8] = (char)rand();
			data->name[9] = '\0';
		}
	}
	ImGui::Separator();
}

#endif

#endif