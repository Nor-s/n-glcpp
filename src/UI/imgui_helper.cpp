#include "imgui_helper.h"
#include <imgui/imgui_internal.h>
#include <imgui/icons/icons.h>
#include <iostream>

namespace ui
{
inline ImVec2 SumImVec2(const ImVec2& a, const ImVec2& b)
{
	return ImVec2{a.x + b.x, a.y + b.y};
}
inline ImVec2 SubImVec2(const ImVec2& a, const ImVec2& b)
{
	return ImVec2{a.x - b.x, a.y - b.y};
}
inline ImVec4 GlmVec4ToImVec4(const glm::vec4& v)
{
	return ImVec4{v.x, v.y, v.z, v.w};
}
void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
bool DragFPropertyXYZ(const char* label,
					  float* value,
					  float step,
					  float min,
					  float max,
					  const char* format,
					  const std::string& help_message,
					  int num)
{
	bool is_value_changed = false;
	ImGui::PushID(label);
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 90.0f);

	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(num, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});

	const char* button_labels[] = {"X", "Y", "Z", "W"};
	const char* drag_labels[] = {"##x", "##y", "##z", "##w"};

	for (int i = 0; i < num; i++)
	{
		auto button_color = glm::vec4{0.0f, 0.0f, 0.0f, 0.5f};
		auto framebg_color = glm::vec4{0.4f, 0.4f, 0.4f, 0.1f};
		auto framebg_hovered_color = glm::vec4{0.4f, 0.4f, 0.4f, 0.7f};
		auto framebg_active_color = glm::vec4{0.5f, 0.5f, 0.5f, 1.0f};

		if (i <= 3)
			button_color[i] = framebg_hovered_color[i] = framebg_active_color[i] = framebg_color[i] = 0.9f;

		ImGui::PushStyleColor(ImGuiCol_Button,
							  GlmVec4ToImVec4(button_color));	 // IM_COL32(button_color.r * 255, button_color.g * 255,
																 // button_color.b * 255, button_color.a * 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
							  GlmVec4ToImVec4(button_color));	 // IM_COL32(button_color.r * 255, button_color.g * 255,
																 // button_color.b * 255, button_color.a * 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
							  GlmVec4ToImVec4(button_color));	 // IM_COL32(button_color.r * 255, button_color.g * 255,
																 // button_color.b * 255, button_color.a * 255));
		ImGui::PushStyleColor(
			ImGuiCol_FrameBgActive,
			GlmVec4ToImVec4(
				framebg_active_color));	   // IM_COL32(framebg_active_color.r * 255, framebg_active_color.g * 255,
										   // framebg_active_color.b * 255, framebg_active_color.a * 255));
		ImGui::PushStyleColor(
			ImGuiCol_FrameBgHovered,
			GlmVec4ToImVec4(
				framebg_hovered_color));	// IM_COL32(framebg_hovered_color.r * 255, framebg_hovered_color.g * 255,
											// framebg_hovered_color.b * 255, framebg_hovered_color.a * 255));
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
		ImGui::Button(button_labels[i]);
		ImGui::PopStyleColor(1);

		ImGui::SameLine();
		is_value_changed |= ImGui::DragFloat(drag_labels[i], &value[i], step, min, max, format);
		ImGui::PopItemWidth();
		if (i != num)
		{
			ImGui::SameLine();
		}
		ImGui::PopStyleColor(5);
	}

	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();

	return is_value_changed;
}

bool DragPropertyXYZ(const char* label,
					 glm::vec3& value,
					 float step,
					 float min,
					 float max,
					 const std::string& help_message)
{
	return DragFPropertyXYZ(label, &value[0], step, min, max, "%.3f", help_message);
}
inline void BeginDragProperty(const char* label, const ImVec2& btn_size)
{
	ImGui::PushID(label);

	ImGuiStyle& style = ImGui::GetStyle();

	auto button_color = style.Colors[ImGuiCol_ButtonHovered];
	button_color.w *= 0.7f;
	auto framebg_color = style.Colors[ImGuiCol_Button];
	auto framebg_hovered_color = style.Colors[ImGuiCol_ButtonHovered];
	auto framebg_active_color = style.Colors[ImGuiCol_ButtonActive];
	ImGui::PushStyleColor(ImGuiCol_Button, button_color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_color);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_color);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, framebg_color);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, framebg_active_color);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, framebg_hovered_color);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
	ImGui::Button(label, btn_size);
	ImGui::SameLine();
}
inline void EndDragProperty()
{
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(6);
	ImGui::PopID();
}
bool DragFloatProperty(const char* label,
					   float& value,
					   float step,
					   float min,
					   float max,
					   const ImVec2& btn_size,
					   const char* format,
					   const std::string& help_message)
{
	bool is_value_changed = false;

	BeginDragProperty(label, btn_size);
	is_value_changed |= ImGui::DragFloat("##drag", &value, step, min, max, format);
	EndDragProperty();

	return is_value_changed;
}
bool DragIntProperty(const char* label,
					 int& value,
					 float step,
					 int min,
					 int max,
					 const ImVec2& btn_size,
					 const std::string& help_message)
{
	bool is_value_changed = false;

	BeginDragProperty(label, btn_size);
	is_value_changed |= ImGui::DragInt("##drag", &value, step, min, max);
	EndDragProperty();

	return is_value_changed;
}

int ToggleButton(const char* label, bool* v, const ImVec2& size, bool* signal)
{
	static int hovered_count = 0;
	ImVec4* colors = ImGui::GetStyle().Colors;
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImGuiContext& g = *GImGui;
	ImVec4 pale_hovered = colors[ImGuiCol_ButtonHovered];
	pale_hovered.w *= 0.5f;
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	const ImGuiStyle& style = g.Style;
	float height =
		(size.y != 0.0f) ? size.y : label_size.y + style.FramePadding.y * 2.0f;				 // ImGui::GetFrameHeight();
	float width = (size.x != 0.0f) ? size.x : label_size.x + style.FramePadding.x * 2.0f;	 // height * 1.55f;
	float rounding = style.FrameRounding;

	ImGui::InvisibleButton(label, ImVec2(width, height));
	if (ImGui::IsItemClicked())
	{
		*v = !*v;
		if (signal)
		{
			*signal = true;
		}
	}
	ImGuiContext& gg = *GImGui;
	float ANIM_SPEED = 0.085f;
	if (gg.LastActiveId == gg.CurrentWindow->GetID(label))	  // && g.LastActiveIdTimer < ANIM_SPEED)
		float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
	if (ImGui::IsItemHovered())
	{
		hovered_count++;
		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height),
								 ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonHovered] : pale_hovered), rounding);
	}
	else
	{
		hovered_count = 0;
		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height),
								 ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : colors[ImGuiCol_Button]),
								 rounding);
	}

	const ImRect bb(p, SumImVec2({width, height}, p));
	if (g.LogEnabled)
		ImGui::LogSetNextTextDecoration("[", "]");
	ImGui::RenderTextClipped(SumImVec2(bb.Min, style.FramePadding), SubImVec2(bb.Max, style.FramePadding), label, NULL,
							 &label_size, style.ButtonTextAlign, &bb);

	return hovered_count;
}
}	 // namespace ui
