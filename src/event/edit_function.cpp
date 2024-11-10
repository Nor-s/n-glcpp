#include "edit_function.h"

#include "entity/entity.h"
#include "entity/components/animation_component.h"
#include "animation/animation.h"
#include "entity/components/pose_component.h"
#include "entity/components/ik_control_component.h"
#include "entity/components/renderable/armature_component.h"

#include "event_history.h"
#include "app.h"

#include <memory>
#include <queue>
#include <glm/glm.hpp>

namespace edit
{

void Change_EntityTransform(anim::Entity* entity, const glm::mat4& new_transform, bool b_is_push_history)
{
	auto& before_transform = entity->get_local();
	if (auto ik_component = entity->get_component<anim::IKControlComponent>(); ik_component != nullptr)
	{
	}
	else
	{
		entity->set_local(new_transform);
		if (auto armature = entity->get_component<anim::ArmatureComponent>(); armature)
		{
			armature->insert_and_update_bone();
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
