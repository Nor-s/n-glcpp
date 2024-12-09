#ifndef ANIM_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H

#include "component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace anim
{
class TransformComponent : public ComponentBase<TransformComponent>
{
public:
	TransformComponent() = default;
	TransformComponent(const glm::mat4& transform);
	glm::mat4 update_mat4() const;
	const glm::mat4& get_mat4() const;
	const glm::vec3& get_translation() const;
	const glm::vec3& get_rotation() const;
	const glm::quat& get_quat() const;
	const glm::vec3& get_scale() const;
	TransformComponent& set_translation(const glm::vec3& vec);
	TransformComponent& set_scale(const glm::vec3& vec);
	TransformComponent& set_scale(float scale);
	TransformComponent& set_rotation(const glm::vec3& vec);
	TransformComponent& set_quat(const glm::quat& quat);
	TransformComponent& set_transform(const glm::mat4& mat);
	TransformComponent& set_transform(const TransformComponent& t);

	glm::mat4 get_relative_transform(const TransformComponent& transfrom) const;

private:
	mutable glm::mat4 transform{1.0f};
	glm::vec3 translation{0.0f, 0.0f, 0.0f};
	glm::vec3 scale{1.f, 1.f, 1.f};
	glm::vec3 rotation{0.0f, 0.0f, 0.0f};
	glm::quat quat{1.0f, 0.0f, 0.0f, 0.0f};
};
}	 // namespace anim
#endif