#include "edit_function.h"

#include "entity/entity.h"
#include "entity/components/animation_component.h"
#include "animation/animation.h"
#include "entity/components/pose_component.h"
#include "entity/components/transform_component.h"
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
	auto axis = glm::cross(s, e);
	auto angle = glm::acos(cos_theta);
	auto degree_angle = anim::ClampAngle(glm::degrees(angle), -20.0, 20.0);
	angle = glm::radians(degree_angle);

	if (cos_theta > 0.999f || glm::length2(axis) <= 0.0f || glm::abs(angle) < 0.00001)
	{
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}

	return glm::normalize(glm::angleAxis(angle, glm::normalize(axis)));
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

	class Chain
	{
	public:
		Chain(anim::ArmatureComponent* a_bone) : bone(a_bone)
		{
			const auto& bone_local = bone->get_owner()->get_local();
			const auto& bone_bindpose = bone->get_bindpose();
			local = anim::TransformComponent(bone_bindpose * bone_local);
			world = anim::TransformComponent(bone->get_owner()->get_world_transformation());
		}
		Chain(const glm::mat4& a_world) : world(a_world)
		{
		}

	public:
		const anim::ArmatureComponent* bone;
		anim::TransformComponent local;
		anim::TransformComponent world;
	};

	// init transform
	std::vector<Chain> chains;
	chains.reserve(8);
	auto temp_entity = start_entity;
	int end_index = -1;
	do
	{
		auto* bone = temp_entity->get_component<anim::ArmatureComponent>();
		if (bone == nullptr)
			break;
		chains.emplace_back(bone);
		if (temp_entity == end_entity)
		{
			end_index = chains.size() - 1;
		}

		temp_entity = temp_entity->get_mutable_parent();
	} while (temp_entity != nullptr && end_index == -1);
	const anim::TransformComponent target(target_world_transform);
	const glm::vec3 target_position = target.get_translation();

	if (temp_entity)
	{
		chains.emplace_back(temp_entity->get_world_transformation());
	}
	else
	{
		chains.emplace_back(glm::mat4(1.0f));
	}

	// calc CCDIK
	int iter = 0;
	double distance = glm::distance(target.get_translation(), chains[ENDEFFECTOR_INDEX].world.get_translation());
	while (distance > 1.0 && iter <= max_iter)
	{
		bool bIsUpdate = false;
		for (int i = 1; i <= end_index; i++)
		{
			Chain& parent_chain = chains[i + 1];
			Chain& current_chain = chains[i];
			Chain& end_chain = chains[ENDEFFECTOR_INDEX];

			// 1. calc axis, angle
			const glm::vec3 to_end =
				glm::normalize(current_chain.world.get_local_point(end_chain.world.get_translation()));
			const glm::vec3 to_target = glm::normalize(current_chain.world.get_local_point(target_position));

			auto delta_rotation = CalcQuat(to_end, to_target);

			if (delta_rotation == glm::quat())
				continue;

			// 2. rotate world, local transform
			current_chain.local.set_quat(glm::normalize(current_chain.local.get_quat() * delta_rotation));
			current_chain.world.set_transform(parent_chain.world.get_mat4() * current_chain.local.get_mat4());

			// 3. update child world transform
			glm::mat4 parent_world_transform = current_chain.world.get_mat4();
			for (int j = i - 1; j >= 0; j--)
			{
				chains[j].world.set_transform(parent_world_transform * chains[j].local.get_mat4());
				parent_world_transform = chains[j].world.get_mat4();
			}
			bIsUpdate = true;
		}
		if (bIsUpdate == false)
		{
			break;
		}
		iter++;
		distance = glm::distance(target.get_translation(), chains[ENDEFFECTOR_INDEX].world.get_translation());
	}

	// update entity transform
	for (int i = 0; i <= end_index; i++)
	{
		const auto diff = glm::inverse(chains[i].bone->get_bindpose()) * chains[i].local.get_mat4();
		Update_EntityTransform(chains[i].bone->get_owner(), diff, b_is_push_history);
	}
}

void Update_ApplyFABRIK(anim::Entity* start_entity,
						const anim::Entity* end_entity,
						const glm::mat4& target_world_transform,
						const uint32_t max_iter = 100u,
						bool b_is_push_history = true)
{
	if (start_entity == nullptr || end_entity == nullptr || start_entity == end_entity)
		return;

	const int ENDEFFECTOR_INDEX = 1;

	class Chain
	{
	public:
		Chain(anim::ArmatureComponent* a_bone) : bone(a_bone), world(glm::vec3(0.0f)), distance(0.0f)
		{
			world = anim::TransformComponent(bone->get_owner()->get_world_transformation()).get_translation();
			distance = glm::length(anim::TransformComponent(a_bone->get_bindpose()).get_translation());
		}
		Chain(const glm::vec3& a_world) : bone(nullptr), world(a_world), distance(0.0f)
		{
		}

	public:
		const anim::ArmatureComponent* bone;
		glm::vec3 world;
		float distance;
	};

	// init transform
	std::vector<Chain> chains;
	chains.reserve(8);
	auto temp_entity = start_entity;
	int end_index = -1;
	const anim::TransformComponent target(target_world_transform);
	const glm::vec3 target_position = target.get_translation();
	chains.emplace_back(target_position);
	do
	{
		auto* bone = temp_entity->get_component<anim::ArmatureComponent>();
		if (bone == nullptr)
			break;
		chains.emplace_back(bone);
		if (temp_entity == end_entity)
		{
			end_index = chains.size() - 1;
		}

		temp_entity = temp_entity->get_mutable_parent();
	} while (temp_entity != nullptr && end_index == -1);
	chains.emplace_back(chains.back().world);

	// calc FABRIK
	int iter = 0;
	float distance = glm::distance(target_position, chains[ENDEFFECTOR_INDEX].world);
	bool bIsUpdate = false;
	while (distance > 1.0f && iter <= max_iter)
	{
		// first phase
		for (int i = 1; i <= end_index; i++)
		{
			auto& current_pos = chains[i].world;
			const auto& before_pos = chains[i - 1].world;
			const float chain_distance = chains[i - 1].distance;

			const glm::vec3 dir = glm::normalize(before_pos - current_pos);
			current_pos = before_pos - chain_distance * dir;
		}

		// second phase
		for (int i = end_index; i >= 1; i--)
		{
			auto& current_pos = chains[i].world;
			const auto& before_pos = chains[i + 1].world;
			const float chain_distance = chains[i].distance;

			const glm::vec3 dir = glm::normalize(before_pos - current_pos);
			current_pos = before_pos - chain_distance * dir;

			if (i == end_index)
			{
				current_pos = before_pos;
			}
		}

		float updated_distance = glm::distance(target_position, chains[ENDEFFECTOR_INDEX].world);
		if (glm::abs(updated_distance - distance) > 1.0f)
		{
			distance = updated_distance;
			bIsUpdate = true;
		}
		else
		{
			break;
		}
		iter++;
	}

	if (!bIsUpdate)
	{
		return;
	}
	// update entity transform
	anim::TransformComponent parent_world_transform(glm::mat4(1.0f));
	if (temp_entity != nullptr)
	{
		parent_world_transform = anim::TransformComponent(temp_entity->get_world_transformation());
	}
	for (int i = end_index; i > 1; i--)
	{
		// child
		const glm::mat4 current_bind = parent_world_transform.get_mat4() * chains[i].bone->get_bindpose();
		anim::TransformComponent current_world_transform = current_bind * chains[i].bone->get_owner()->get_local();
		const glm::mat4 child_transform = current_world_transform.get_mat4() * chains[i - 1].bone->get_bindpose() *
										  chains[i - 1].bone->get_owner()->get_local();

		const glm::vec3 current = anim::TransformComponent(current_world_transform).get_translation();
		const glm::vec3 to_target = chains[i - 1].world - current;
		const glm::vec3 to_child = anim::TransformComponent(child_transform).get_translation() - current;
		auto delta_rotation = CalcQuat(glm::normalize(to_child), glm::normalize(to_target));

		current_world_transform.set_quat(glm::normalize(delta_rotation * current_world_transform.get_quat()));
		const glm::mat4 relative_transform = parent_world_transform.get_relative_transform(current_world_transform);
		const glm::mat4 diff = glm::inverse(chains[i].bone->get_bindpose()) * relative_transform;

		Update_EntityTransform(chains[i].bone->get_owner(), diff, b_is_push_history);

		// App::get_instance()->get_current_scene()->draw_debug(DebugInfo{glm::translate(glm::mat4(1.0f),
		// chains[i].world)});

		parent_world_transform.set_transform(current_world_transform);
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
			auto type = ik_component->get_ik_type();
			auto max_iter = ik_component->get_max_iter();
			auto target_transform = ik_component->get_world_transformation();
			switch (type)
			{
				case anim::IKType::CCDIK:
					Update_ApplyCCDIK(start_entity, end_entity, target_transform, max_iter, b_is_push_history);
					break;
				case anim::IKType::FABRIK:
					Update_ApplyFABRIK(start_entity, end_entity, target_transform, max_iter, b_is_push_history);
					break;
			}
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
