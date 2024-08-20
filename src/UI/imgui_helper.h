#ifndef UI_IMGUI_HELPER_H
#define UI_IMGUI_HELPER_H

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <string>
namespace ui
{
inline ImVec2 SumImVec2(const ImVec2& a, const ImVec2& b);
inline ImVec2 SubImVec2(const ImVec2& a, const ImVec2& b);
inline ImVec4 GlmVec4ToImVec4(const glm::vec4& v);

void HelpMarker(const char* desc);
inline void BeginDragProperty(const char* label, const ImVec2& btn_size = {0.0f, 0.0f});
inline void EndDragProperty(const char* label);
bool DragFPropertyXYZ(const char* label,
					  float* value,
					  float step = 0.01f,
					  float min = -1000.0f,
					  float max = 1000.0f,
					  const char* format = "%.2f",
					  const std::string& help_message = "",
					  int num = 3);
bool DragPropertyXYZ(const char* label,
					 glm::vec3& value,
					 float step = 0.01f,
					 float min = -1000.0f,
					 float max = 1000.0f,
					 const std::string& help_message = "");
bool DragFloatProperty(const char* label,
					   float& value,
					   float step = 1.0f,
					   float min = -1000.0f,
					   float max = 1000.0f,
					   const ImVec2& btn_size = {0.0f, 0.0f},
					   const char* format = "%.0f",
					   const std::string& help_message = "");
bool DragIntProperty(const char* label,
					 int& value,
					 float step = 1,
					 int min = 0,
					 int max = 1000,
					 const ImVec2& btn_size = {0.0f, 0.0f},
					 const std::string& help_message = "");
int ToggleButton(const char* label, bool* v, const ImVec2& size = {0.0f, 0.0f}, bool* signal = nullptr);
}	 // namespace ui

#endif