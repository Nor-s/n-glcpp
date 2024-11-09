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

}	 // namespace anim
