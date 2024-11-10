#ifndef ANIM_EVENT_EDIT_EVENT_H
#define ANIM_EVENT_EDIT_EVENT_H

#include "components/transform_component.h"

namespace anim
{
class Entity;
}

namespace edit
{

void Change_EntityTransform(anim::Entity* entity, const glm::mat4& transform, bool b_is_push_history);
void Delete_AnimationFrame(anim::Entity* entity, bool b_is_push_history = true);

}	 // namespace edit

#endif
