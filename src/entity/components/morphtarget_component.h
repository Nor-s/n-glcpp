#ifndef ANIM_ENTITY_COMPONENT_MORPH_TARGET_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_MORPH_TARGET_COMPONENT_H

#include "../component.h"

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace anim
{
class Entity;
class MorphTarget;

class MorphTargetComponent : public ComponentBase<MorphTargetComponent>
{
public:
private:
	std::vector<std::shared_ptr<MorphTarget>> meshes_;
	Entity* root_entity_;
};
}	 // namespace anim

#endif