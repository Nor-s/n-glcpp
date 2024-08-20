#ifndef ANIM_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H

#include "component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace anim
{
class TransformComponent : public ComponentBase<TransformComponent>
{
public:
	glm::mat4 get_mat4() const;
	const glm::vec3& get_translation() const;
	const glm::vec3& get_rotation() const;
	const glm::vec3& get_scale() const;
	TransformComponent& set_translation(const glm::vec3& vec);
	TransformComponent& set_scale(const glm::vec3& vec);
	TransformComponent& set_scale(float scale);
	TransformComponent& set_rotation(const glm::vec3& vec);
	TransformComponent& set_transform(const glm::mat4& mat);
	glm::vec3 mTranslation{0.0f, 0.0f, 0.0f};
	glm::vec3 mScale{1.f, 1.f, 1.f};
	glm::vec3 mRotation{0.0f, 0.0f, 0.0f};

private:
};
}	 // namespace anim
#endif