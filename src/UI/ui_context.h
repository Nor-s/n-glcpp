#ifndef ANIM_UI_UI_CONTEXT
#define ANIM_UI_UI_CONTEXT

#include <string>
#include <glm/glm.hpp>

namespace ui
{
// Change animation
struct ComponentContext
{
	int current_animation_idx{-1};
	int new_animation_idx{-1};
	bool is_changed_animation{false};
	bool is_clicked_retargeting{false};
	ComponentContext() : current_animation_idx(-1), new_animation_idx(-1), is_changed_animation(false)
	{
	}
};

// Change animator status (play/pause, fps, current time, start time, end time)
struct TimelineContext
{
	bool is_recording{false};
	bool is_clicked_play_back{false};
	bool is_clicked_play{false};
	bool is_stop{false};
	bool is_current_frame_changed{false};
	float fps{1.0f};
	int start_frame{0};
	int end_frame{200};
	int current_frame{0};
	bool is_forward{false};
	bool is_backward{false};
	bool is_delete_current_frame{false};
	TimelineContext()
		: is_recording(false)
		, is_clicked_play_back(false)
		, is_clicked_play(false)
		, is_stop(false)
		, is_current_frame_changed(false)
		, fps(1.0f)
		, start_frame(0)
		, end_frame(200)
		, current_frame(0)
		, is_forward(false)
		, is_backward(false)
		, is_delete_current_frame(false)
	{
	}
};

// import, export
struct MenuContext
{
	bool is_clicked_import_model{false};
	bool is_clicked_import_dir{false};
	bool is_clicked_export_animation{false};
	bool is_clicked_export_all_data{false};
	bool is_dialog_open{false};
	bool is_export_linear_interpolation{true};
	float import_scale{1.0f};
	std::string path{""};
	MenuContext()
		: is_clicked_import_model(false)
		, is_clicked_import_dir(false)
		, is_clicked_export_animation(false)
		, is_clicked_export_all_data(false)
		, is_dialog_open(false)
		, is_export_linear_interpolation(true)
		, import_scale(1.0f)
	{
	}
};

// node change (bone, entity)
struct EntityContext
{
	bool is_changed_selected_entity{false};
	bool is_changed_transform{false};
	bool is_manipulated{false};
	int selected_id{-1};
	glm::mat4 new_transform{1.0f};
	EntityContext()
		: is_changed_selected_entity(false)
		, is_changed_transform(false)
		, is_manipulated(false)
		, selected_id(-1)
		, new_transform(1.0f)
	{
	}
};

struct SceneContext
{
	bool is_picking{false};
	bool is_bone_picking_mode{false};
	bool is_clicked_picking_mode{false};
	int x{0};
	int y{0};
	SceneContext() : is_picking(false), is_bone_picking_mode(false), is_clicked_picking_mode(false), x(0), y(0)
	{
	}
};

struct PythonContext
{
	static inline float min_visibility{0.0f};
	static inline float min_detection_confidence{0.5f};
	static inline int model_complexity{1};
	static inline bool is_angle_adjustment{false};
	static inline bool is_holistic{false};
	static inline float fps{24.0f};
	static inline float factor{0.0f};
	bool is_clicked_convert_btn{false};
	std::string save_path;
	std::string video_path;
	PythonContext() : is_clicked_convert_btn(false), save_path(""), video_path("")
	{
	}
};

struct UiContext
{
	MenuContext menu{};
	TimelineContext timeline{};
	ComponentContext component{};
	EntityContext entity{};
	SceneContext scene{};
	PythonContext python{};
	UiContext() : menu(), timeline(), component(), entity(), scene(), python()
	{
	}
};
}	 // namespace ui

#endif