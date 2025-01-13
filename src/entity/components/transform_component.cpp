#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "transform_component.h"
#include "../../util/utility.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace anim
{
TransformComponent::TransformComponent(const glm::mat4& transform)
{
	set_transform(transform);
}
glm::mat4 TransformComponent::update_mat4() const
{
	glm::mat4 rotation = glm::mat4(quat);

	transform = glm::translate(glm::mat4(1.0f), translation) * rotation * glm::scale(glm::mat4(1.0f), scale);
	return transform;
}
const glm::mat4& TransformComponent::get_mat4() const
{
	return transform;
}
const glm::vec3& TransformComponent::get_translation() const
{
	return translation;
}
const glm::vec3& TransformComponent::get_rotation() const
{
	return rotation;
}
const glm::quat& TransformComponent::get_quat() const
{
	return quat;
}
const glm::vec3& TransformComponent::get_scale() const
{
	return scale;
}
TransformComponent& TransformComponent::set_translation(const glm::vec3& vec)
{
	translation = vec;
	update_mat4();
	return *this;
}
TransformComponent& TransformComponent::set_scale(const glm::vec3& vec)
{
	scale = vec;
	update_mat4();
	return *this;
}
TransformComponent& TransformComponent::set_scale(float scale)
{
	set_scale(glm::vec3{scale, scale, scale});
	update_mat4();
	return *this;
}
TransformComponent& TransformComponent::set_rotation(const glm::vec3& vec)
{
	rotation = vec;
	quat = glm::normalize(glm::quat(rotation));
	update_mat4();
	return *this;
}
TransformComponent& TransformComponent::set_quat(const glm::quat& q)
{
	quat = glm::normalize(q);
	rotation = glm::eulerAngles(q);
	update_mat4();
	return *this;
}
TransformComponent& TransformComponent::set_transform(const glm::mat4& mat)
{
	transform = mat;
	auto [t, r, s] = DecomposeTransform(mat);
	translation = t;
	quat = r;
	rotation = glm::eulerAngles(r);
	scale = s;
	return *this;
}
TransformComponent& TransformComponent::set_transform(const TransformComponent& t)
{
	return set_transform(t.transform);
}

glm::mat4 TransformComponent::get_relative_transform(const TransformComponent& t) const
{
	return glm::inverse(get_mat4()) * t.transform;
}

glm::vec3 TransformComponent::get_local_point(const glm::vec3& world_point) const
{
	auto b = world_point - get_translation();
	auto A = glm::mat3(get_mat4());
	auto x = glm::inverse(A) * b;
	if (glm::isnan(x.x))
		x.x = 0.0;

	if (glm::isnan(x.y))
		x.y = 0.0;

	if (glm::isnan(x.z))
		x.z = 0.0;

	return x;
}

}	 // namespace anim