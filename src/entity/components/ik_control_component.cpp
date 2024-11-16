#include "ik_control_component.h"
#include "ik_control_component.h"
#include "ik_control_component.h"
#include "entity/entity.h"

namespace anim
{
void IKControlComponent::set_end(Entity* end_entity)
{
	if (end_entity == nullptr || get_owner() == end_entity)
		return;

	end_entity_ = end_entity;
}
const glm::mat4& anim::IKControlComponent::get_world_transformation() const
{
	if (!is_snapping())
	{
		world_transform_ = get_owner()->get_world_transformation();
	}
	return world_transform_;
}
}	 // namespace anim
