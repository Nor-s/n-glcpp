#include "edit_function.h"

#include "entity/entity.h"
#include "entity/components/animation_component.h"
#include "animation/animation.h"
#include "entity/components/pose_component.h"
#include "entity/components/ik_control_component.h"
#include "entity/components/renderable/armature_component.h"
#include "util/utility.h"

#include "event_history.h"
#include "app.h"

#include <memory>
#include <queue>
#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <chrono>
#include <thread>

#include <cmath>

namespace edit
{

const glm::quat CalcQuat(glm::vec3 s, glm::vec3 e)
{
	auto cos_theta = glm::dot(s, e);
	auto axis = glm::normalize(glm::cross(s, e));

	if (cos_theta > 0.999f)
	{
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);	 // 단위 쿼터니언
	}
	if (cos_theta < -0.99f)
	{
		return glm::normalize(glm::angleAxis(glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	return glm::normalize(glm::angleAxis(glm::acos(cos_theta), axis));

	// if (cos_theta < -1.0 + 0.001)
	//{
	//	axis = glm::cross(glm::vec3(0.0, 0.0, 1.0), s);
	//	if (glm::length2(axis) < 0.01)
	//	{
	//		axis = glm::cross(glm::vec3(1.0, 0.0, 0.0), s);
	//	}
	//	axis = glm::normalize(axis);
	//	return glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(axis));
	// }
	// float ss = std::sqrt((1.0f + cos_theta) * 2.0f);
	// float invs = 1.0f / ss;
	// glm::quat quat(ss * 0.5f, axis.x * invs, axis.y * invs, axis.z * invs);

	// return glm::toMat4(glm::normalize(quat));
}

const glm::vec3 GetTranslate(const glm::mat4& mat)
{
	return mat[3];
}

void Update_EntityTransform(anim::Entity* entity, const glm::mat4& new_transform, bool b_is_push_history)
{
	auto& before_transform = entity->get_local();
	auto armature = entity->get_component<anim::ArmatureComponent>();

	entity->set_local(new_transform);
	if (armature != nullptr)
	{
		armature->insert_and_update_bone();
	}

	// TODO: push history when armature is nullptr
	if (b_is_push_history && armature)
	{
		auto* app = App::get_instance();
		auto* scene = app->get_current_scene();

		app->history_push(std::make_unique<anim::BoneChangeEvent>(
			scene, scene->get_mutable_ref_shared_resources().get(), entity->get_id(),
			entity->get_mutable_root()->get_component<anim::AnimationComponent>()->get_animation()->get_id(),
			before_transform, armature->get_pose()->get_animator()->get_current_time()));
	}
}

/*
must start entity is child of end entity
*/
void Update_ApplyCCDIK(anim::Entity* start_entity,
					   const anim::Entity* end_entity,
					   const glm::mat4& target_world_transform,
					   const uint32_t max_iter = 100u,
					   bool b_is_push_history = true)
{
	if (start_entity == nullptr || end_entity == nullptr || start_entity == end_entity)
		return;

	const int ENDEFFECTOR_INDEX = 0;

	// store init transform
	std::vector<glm::quat> local_rotations;
	std::vector<glm::mat4> local_translate_and_scale;
	std::vector<glm::mat4> temp_world_transforms;
	std::vector<anim::ArmatureComponent*> bones;
	local_rotations.reserve(8);
	temp_world_transforms.reserve(8);
	auto temp_entity = start_entity;
	int end_index = 0;
	do
	{
		auto* armature = temp_entity->get_component<anim::ArmatureComponent>();
		if (armature == nullptr)
			break;

		bones.emplace_back(armature);

		temp_entity = temp_entity->get_mutable_parent();
		if (temp_entity == end_entity)
		{
			end_index = bones.size() - 1;
		}
	} while (temp_entity != nullptr);
	std::reverse(bones.begin(), bones.end());

	// calc world transform
	auto get_world = [](const glm::mat4& parent_world_transform, anim::ArmatureComponent* armature,
						const glm::mat4& local) { return parent_world_transform * armature->get_bindpose() * local; };

	const int size = bones.size();
	auto bone_local = glm::mat4(1.0f);
	auto parent = glm::mat4(1.0f);
	for (int i = 0; i < size; i++)
	{
		bone_local = bones[i]->get_owner()->get_local();
		if (i > 0)
			parent = temp_world_transforms[i - 1];
		temp_world_transforms.emplace_back(get_world(parent, bones[i], bone_local));
		auto [t, r, s] = anim::DecomposeTransform(bone_local);
		local_rotations.emplace_back(glm::normalize(r));
		local_translate_and_scale.emplace_back(glm::translate(glm::mat4(1.0f), t) * glm::scale(glm::mat4(1.0f), s));
	}
	const glm::vec3 target_vector = target_world_transform[3];
	anim::LOG(glm::to_string(target_vector));
	assert(size >= 2);

	// calc CCDIK
	std::vector<glm::quat> rotate_matrix(size, glm::quat());
	std::reverse(temp_world_transforms.begin(), temp_world_transforms.end());
	std::reverse(local_rotations.begin(), local_rotations.end());
	std::reverse(bones.begin(), bones.end());
	for (int32_t iter = 0; iter < 1000; iter++)
	{
		for (int i = 1; i <= end_index; i++)
		{
			const glm::vec3 root_vector = GetTranslate(temp_world_transforms[i]);
			const glm::vec3 end_effector_vector = GetTranslate(temp_world_transforms[ENDEFFECTOR_INDEX]);

			const glm::vec3 root_to_end_effector = glm::normalize(end_effector_vector - root_vector);
			const glm::vec3 root_to_target = glm::normalize(target_vector - root_vector);

			// rotate_matrix[i] = glm::rotate(glm::mat4(1.0f), angle, cross);
			rotate_matrix[i] =
				CalcQuat(root_to_end_effector, root_to_target);	   // glm::rotate(glm::mat4(1.0f), angle, cross);
			local_rotations[i] = glm::normalize(local_rotations[i] * rotate_matrix[i]);
			glm::mat4 parent_world_transform = glm::mat4(1.0f);
			for (int j = i; j >= 0; j--)
			{
				if (temp_world_transforms.size() > j + 1)
					parent_world_transform = temp_world_transforms[j + 1];
				temp_world_transforms[j] = get_world(parent_world_transform, bones[j], glm::toMat4(local_rotations[j]));
			}
		}
	}

	for (int i = 0; i < size; i++)
	{
		Update_EntityTransform(bones[i]->get_owner(), glm::toMat4(local_rotations[i]), b_is_push_history);
	}
}

void Update_EntitiesTransform(anim::Entity* entity, const glm::mat4& new_transform, bool b_is_push_history)
{
	auto& before_transform = entity->get_local();
	auto armature = entity->get_component<anim::ArmatureComponent>();

	if (auto ik_component = entity->get_component<anim::IKControlComponent>(); ik_component != nullptr)
	{
		bool b_is_translate = new_transform[3][0] != 0.0f || new_transform[3][1] != 0.0f && new_transform[3][2] != 0.0f;

		if (b_is_translate)
		{
			auto* start_entity = ik_component->get_owner();
			auto* end_entity = ik_component->get_end();

			Update_ApplyCCDIK(start_entity, end_entity, ik_component->get_world_transformation(),
							  ik_component->get_max_iter(), b_is_push_history);
			return;
		}
	}

	Update_EntityTransform(entity, new_transform, b_is_push_history);
}

void Delete_AnimationFrame(anim::Entity* entity, bool b_is_push_history)
{
	anim::LOG("delete current bone");
	auto& before_transform = entity->get_local();
	if (auto armature = entity->get_component<anim::ArmatureComponent>(); armature)
	{
		armature->get_pose()->sub_current_bone(armature->get_name());

		if (b_is_push_history)
		{
			auto* app = App::get_instance();
			auto* scene = app->get_current_scene();

			app->history_push(std::make_unique<anim::BoneChangeEvent>(
				scene, scene->get_mutable_ref_shared_resources().get(), entity->get_id(),
				entity->get_mutable_root()->get_component<anim::AnimationComponent>()->get_animation()->get_id(),
				before_transform, armature->get_pose()->get_animator()->get_current_time()));
		}
	}
}

}	 // namespace edit
