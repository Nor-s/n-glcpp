#include "armature_component.h"
#include "../pose_component.h"
#include "../../../graphics/shader.h"
#include "../../entity.h"
#include "../../graphics/mesh.h"
#include "../transform_component.h"
#include "../../../graphics/opengl/gl_mesh.h"
#include "../../../util/utility.h"

namespace anim
{
std::unique_ptr<Mesh> ArmatureComponent::armature_ = nullptr;
void ArmatureComponent::setShape(std::unique_ptr<Mesh> shape)
{
	armature_ = std::move(shape);
}

void ArmatureComponent::set_pose(PoseComponent* pose)
{
	pose_ = pose;
}
void ArmatureComponent::set_name(const std::string& name)
{
	name_ = name;
}
void ArmatureComponent::set_bone_id(int id)
{
	id_ = id;
}
void ArmatureComponent::set_bone_offset(const glm::mat4& offset)
{
	offset_ = offset;
}
void ArmatureComponent::set_bind_pose(const glm::mat4& bind_pose)
{
	bind_pose_ = bind_pose;
}

void ArmatureComponent::set_model_pose(const glm::mat4& model_pose)
{
	model_ = model_pose;
}
PoseComponent* ArmatureComponent::get_pose()
{
	return pose_;
}
const glm::mat4& ArmatureComponent::get_bone_offset() const
{
	return offset_;
}
const glm::mat4& ArmatureComponent::get_bindpose() const
{
	return bind_pose_;
}
void ArmatureComponent::set_local_scale(int idx, float scale)
{
	if (scale_.size() <= idx)
	{
		scale_.emplace_back(1.0f);
	}
	scale_[idx] = scale;
}
// http : // www.opengl-tutorial.org/kr/intermediate-tutorials/tutorial-17-quaternions/

void ArmatureComponent::set_local_rotation_with_target(int idx, const glm::vec3& target)
{
	glm::vec3 origin = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 t = glm::normalize(target);
	float cos_theta = glm::dot(origin, t);
	glm::vec3 axis = glm::cross(origin, t);
	if (cos_theta < -1 + 0.001f)
	{
		axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), origin);
		if (glm::length2(axis) < 0.01f)
		{
			axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), origin);
		}
		axis = glm::normalize(axis);
		set_local_rotation(idx, glm::angleAxis(glm::radians(180.0f), axis));
	}
	else
	{
		float s = sqrt((1 + cos_theta) * 2);
		float invs = 1 / s;

		set_local_rotation(idx, glm::quat(s * 0.5f, axis.x * invs, axis.y * invs, axis.z * invs));
	}
}
void ArmatureComponent::set_local_rotation(int idx, const glm::quat& quat)
{
	if (rotation_.size() <= idx)
	{
		rotation_.emplace_back(glm::quat{});
	}
	rotation_[idx] = quat;
}
const std::string& ArmatureComponent::get_name() const
{
	return name_;
}
float ArmatureComponent::get_local_scale()
{
	return scale_.back();
}
int ArmatureComponent::get_id()
{
	return id_;
}
const glm::mat4& ArmatureComponent::get_model() const
{
	return model_;
}
const glm::quat& ArmatureComponent::get_local_rotation()
{
	return rotation_.back();
}
void ArmatureComponent::set_shader(Shader* shader)
{
	shader_ = shader;
}

void ArmatureComponent::set_entity(Entity* entity)
{
	entity_ = entity;
}
void ArmatureComponent::update()
{
	auto world = pose_->get_root_entity()->get_mutable_parent()->get_world_transformation();
	shader_->use();
	shader_->set_uint("selectionColor", selectionColor);
	world = world * model_;
	entity_->set_world_transformation(world);
	if (!isActivate)
	{
		return;
	}
	auto& mat = armature_->get_mutable_mat_properties();

	for (int i = 0; i < rotation_.size(); i++)
	{
		mat.diffuse =
			glm::mix(glm::vec3{0.87f, 0.87f, 0.97f}, glm::vec3{0.0f, 0.0f, 0.0f}, static_cast<float>(id_) / 128.0f);
		mat.shininess = 1.0f;
		glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(scale_[i]));
		//	if (isApplayLocalRotation)
		{
			s = glm::mat4(rotation_[i]) * s;
		}
		shader_->set_mat4("model", world * s);
		if (entity_->is_selected_ || entity_->get_mutable_root()->is_selected_ ||
			pose_->get_root_entity()->get_mutable_parent()->is_selected_)
		{
			mat.shininess = 100.0f;
			mat.diffuse = glm::vec4{1.0f, 0.5f, 0.02f, 1.0f};

			armature_->draw_outline(*shader_);
		}
		else
		{
			armature_->draw(*shader_);
		}
	}
}
void ArmatureComponent::insert_and_update_bone()
{
	pose_->insert_and_update_bone(name_, entity_->get_local());
}
}	 // namespace anim