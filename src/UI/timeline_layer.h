#ifndef UI_IMGUI_TIMELINE_LAYER_H
#define UI_IMGUI_TIMELINE_LAYER_H

#include <vector>
#include <memory>
#include <string>
#include "ui_context.h"

class Scene;

namespace anim
{
class Entity;
class SharedResources;
class Animation;
class Animator;
class AnimationComponent;
class Bone;
}	 // namespace anim

namespace ui
{
class TextEditLayer;

class TimelineLayer
{
public:
	TimelineLayer();
	~TimelineLayer() = default;
	void draw(Scene* scene, UiContext& context);

private:
	inline void init_context(UiContext& ui_context, Scene* scene);
	void draw_animator_status(UiContext& ui_context);

	// TODO: Fix neo sequencer error when start frame is not 0"
	void draw_sequencer(UiContext& ui_context);
	void draw_keyframes(UiContext& ui_context, const anim::Animation* animation);
	void draw_keyframe_popup(UiContext& ui_context);
	bool draw_drag_popup();

	std::shared_ptr<TextEditLayer> text_editor_;
	bool is_hovered_zoom_slider_{false};
	bool is_opened_transform_{true};
	bool is_recording_{true};
	uint32_t current_frame_{0u};
	Scene* scene_;
	anim::Entity* root_entity_;
	anim::Entity* entity_;
	anim::SharedResources* resources_;
	anim::Animator* animator_;
};
}	 // namespace ui

#endif