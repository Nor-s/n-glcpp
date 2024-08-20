#ifndef ANIM_EVENT_EVENT_HISTORY_H
#define ANIM_EVENT_EVENT_HISTORY_H

#include <glm/glm.hpp>
#include <resources/shared_resources.h>
#include "../scene/scene.hpp"
#include "../entity/components/animation_component.h"
#include "../animation/animation.h"
#include "../entity/components/pose_component.h"
#include <queue>

namespace anim
{
class Animation;
class Entity;
class Bone;
class Event
{
public:
	virtual ~Event() = default;
	virtual void revoke() = 0;
};
class BoneChangeEvent : public Event
{
public:
	BoneChangeEvent() = delete;
	BoneChangeEvent(Scene* scene,
					SharedResources* resources,
					int entity_id,
					int anim_id,
					const glm::mat4& before_transform,
					float time)
		: scene_(scene)
		, resources_(resources)
		, entity_id_(entity_id)
		, anim_id_(anim_id)
		, before_transform_(before_transform)
		, time_(time)
	{
	}

	// TODO: animation fps save to animation (not animation component)
	void revoke()
	{
		scene_->set_selected_entity(entity_id_);

		auto entity = scene_->get_mutable_selected_entity();
		auto root_entity = entity->get_mutable_root();
		auto anim_component = root_entity->get_component<AnimationComponent>();
		auto anim = resources_->get_mutable_animation(anim_id_);
		if (anim_component->get_animation()->get_id() != anim_id_)
		{
			anim_component->set_animation(anim);
		}
		anim_component->set_animation(anim);
		anim->update_keyframe(entity->get_name(), before_transform_, time_);
		root_entity->get_component<PoseComponent>()->get_animator()->set_current_time(time_);
	}

private:
	Scene* scene_;
	SharedResources* resources_;
	int entity_id_{-1};
	int anim_id_{-1};
	glm::mat4 before_transform_{};
	float time_{-1.0f};
};

class EventHistoryQueue
{
public:
	void push(std::unique_ptr<Event> event)
	{
		if (history_.size() >= max_size_)
		{
			history_.pop_front();
		}
		history_.push_back(std::move(event));
	}
	void pop()
	{
		if (history_.size() > 0)
		{
			history_.back()->revoke();
			history_.pop_back();
		}
	}

private:
	size_t max_size_ = 1000;
	std::deque<std::unique_ptr<Event>> history_;
};
};	  // namespace anim
#endif