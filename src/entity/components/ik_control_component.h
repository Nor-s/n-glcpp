#ifndef ANIM_ENTITY_COMPONENT_IK_CONTROL_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_IK_CONTROL_COMPONENT_H

#include "component.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace anim
{
class Entity;

class IKControlComponent : public ComponentBase<IKControlComponent>
{
public:
	void set_end(Entity* end_entity);
	const Entity* get_end() const
	{
		return end_entity_;
	}
	const uint32_t get_max_iter() const
	{
		return max_iter_;
	}

	void set_max_iter(const uint32_t max_iter)
	{
		max_iter_ = max_iter;
	}
	void set_snapping(bool b_is_snapping)
	{
		b_is_snapping_ = b_is_snapping;
	}
	bool is_snapping() const
	{
		return b_is_snapping_;
	}
	const glm::mat4& get_world_transformation() const;

	void set_world_transformation(const glm::mat4& transform)
	{
		world_transform_ = transform;
	}

private:
	mutable glm::mat4 world_transform_;
	Entity* end_entity_{nullptr};
	uint32_t max_iter_ = 100;
	bool b_is_snapping_ = false;
};

}	 // namespace anim
#endif