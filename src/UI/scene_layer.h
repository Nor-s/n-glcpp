#ifndef UI_IMGUI_SCENE_WINDOW_H
#define UI_IMGUI_SCENE_WINDOW_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include "scene/scene.hpp"
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui_context.h"
namespace ui
{
class SceneLayer
{
public:
	SceneLayer();
	~SceneLayer() = default;
	void draw(const char* title, Scene* scene, UiContext& ui_context);
	float get_width();
	float get_height();
	float get_aspect();
	bool get_is_hovered();

private:
	void draw_gizmo(Scene* scene, UiContext& ui_context);
	void draw_mode_window(UiContext& ui_context);
	float width_{800.0f};
	float height_{800.0f};
	bool is_hovered_{false};
	bool is_bone_picking_mode_{true};
	bool is_univ_mode_{false};
	bool is_rotate_mode_{false};
	bool is_scale_mode_{false};
	bool is_translate_mode_{false};
	float scene_window_right_{0.0f};
	float scene_window_top_{0.0f};
	ImVec2 scene_pos_{0.0f, 0.0f};
	ImGuizmo::MODE current_gizmo_mode_;
	ImGuizmo::OPERATION current_gizmo_operation_;
	float scene_cursor_y_{0.0f};
};
}	 // namespace ui

#endif
