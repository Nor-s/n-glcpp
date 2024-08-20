#ifndef ANIM_ENTITY_COMPONENT_ANIMATION_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_ANIMATION_COMPONENT_H

#include "component.h"
#include <memory>

namespace anim
{
class Animation;

class AnimationComponent : public ComponentBase<AnimationComponent>
{
public:
	void init_animation();
	void play();
	void stop();
	void reload();
	void set_animation(Animation* animation);
	void set_current_frame_num_to_time(uint32_t frame);
	void set_custom_tick_per_second(float tick_per_second);
	void set_fps(float fps);
	const Animation* get_animation() const;
	Animation* get_mutable_animation();
	float get_origin_current_time(float time);
	bool* get_mutable_pointer_is_loop();
	bool& get_mutable_is_loop();
	bool& get_mutable_is_stop();
	const uint32_t get_current_frame_num() const;
	const uint32_t get_custom_duration() const;
	float& get_mutable_custom_tick_per_second();
	float get_ticks_per_second_factor() const;
	float& get_mutable_current_time();
	float& get_mutable_fps();
	float get_fps() const;
	float get_tps() const;

private:
	Animation* animation_{nullptr};
	float current_time_ = 0.0f;
	float fps_ = 24.0f;
	float custom_ticks_per_second_ = 24.0f;
	bool is_stop_ = false;
	bool is_loop_ = true;
};
}	 // namespace anim

#endif