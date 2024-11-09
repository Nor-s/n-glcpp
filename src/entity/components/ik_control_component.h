#ifndef ANIM_ENTITY_COMPONENT_IK_CONTROL_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_IK_CONTROL_COMPONENT_H

#include "component.h"
#include <string>
#include <vector>

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

private:
	Entity* end_entity_{nullptr};
};

}	 // namespace anim
#endif