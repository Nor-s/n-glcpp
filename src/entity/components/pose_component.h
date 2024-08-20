#ifndef ANIM_ENTITY_COMPONENT_POSE_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_POSE_COMPONENT_H

#include "component.h"
#include "../../resources/model.h"
#include "../../animation/animator.h"
#include "../../graphics/shader.h"

#include <unordered_map>

namespace anim
{
	class AnimationComponent;
	class PoseComponent : public ComponentBase<PoseComponent>
	{
	public:
		PoseComponent();

		Entity* get_root_entity();
		Animator* get_animator();
		AnimationComponent* get_animation_component();

		void set_bone_info_map(std::unordered_map<std::string, BoneInfo>& bone_info_map);
		void set_animator(Animator* animator);
		void set_animation_component(AnimationComponent* animation_component);
		void set_shader(Shader* shader);
		void set_armature_root(Entity* armature_root);

		void add_bone(const std::string& name, BoneInfo info);
		void sub_current_bone(const std::string& name);
		void update() override;
		void insert_and_update_bone(const std::string& name, const glm::mat4& transform);

		Entity* find(int bone_id);
		Entity* find(int bone_id, Entity* entity);

	private:
		std::unordered_map<std::string, BoneInfo> bone_info_map_{};
		Shader* shader_{ nullptr };
		Animator* animator_{ nullptr };
		AnimationComponent* animation_component_{ nullptr };
		Entity* armature_root_{ nullptr };
	};

}

#endif