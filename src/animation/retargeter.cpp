#include "retargeter.h"
#include "animation.h"
#include "bone.h"
#include "json_animation.h"

#include "../entity/components/pose_component.h"
#include "../entity/components/animation_component.h"
#include "../entity/components/renderable/armature_component.h"
#include "../entity/entity.h"
#include "../util/utility.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <set>

namespace anim
{
void MixamoRetargeter::init_animation_temp_node(Entity* entity,
												AnimationTempNode& anim_node,
												AnimationTempNode* parent_node)
{
	auto arm_comp = entity->get_component<ArmatureComponent>();
	if (!arm_comp)
	{
		return;
	}
	const std::string& name = arm_comp->get_name();
	const glm::mat4& bindpose = arm_comp->get_bindpose();
	auto [t, r, s] = DecomposeTransform(bindpose);

	anim_node.NAB_trans = t;
	anim_node.NAB_rotat = r;
	anim_node.NAB_scale = s;

	if (auto it = animation_bindpose_.find(name); it != animation_bindpose_.end())
	{
		anim_node.name = name;
		anim_node.exist_bone = true;
		anim_node.AB = it->second;
		auto [at, ar, as] = DecomposeTransform(anim_node.AB);
		anim_node.NAB_trans = at;	 // for hips pos
		anim_node.NAB_scale = as;	 // for hips pos
		anim_node.world_transform *= anim_node.AB;
		glm::vec4 current_world_pos = anim_node.world_transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		if (parent_node)
		{
			anim_node.NAB_trans = GetRelativePos(current_world_pos, parent_node->n_world_transform);
		}

		auto new_bindpose = ComposeTransform(anim_node.NAB_trans, anim_node.NAB_rotat, anim_node.NAB_scale);
		anim_node.n_world_transform *= new_bindpose;
		auto new_pos = anim_node.n_world_transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
#ifndef NDEBUG
		std::cout << name << ": " << glm::to_string(current_world_pos) << " : " << glm::to_string(new_pos) << std::endl;
#endif

		new_animation_bindpose_[name] = new_bindpose;
		animation_bindpose_offset_[name] = glm::inverse(anim_node.world_transform);
		new_animation_bindpose_offset_[name] = glm::inverse(anim_node.n_world_transform);
	}

	// dfs
	auto& children = entity->get_mutable_children();
	anim_node.children = std::vector<AnimationTempNode>(children.size());
	for (int i = 0; i < children.size(); ++i)
	{
		auto& child = children[i];
		auto& anim_child = anim_node.children[i];
		anim_child.world_transform = anim_node.world_transform;
		anim_child.n_world_transform = anim_node.n_world_transform;
		init_animation_temp_node(child.get(), anim_child, &anim_node);
	}
}
void MixamoRetargeter::init_animation_bindpose(const Animation* anim)
{
	// auto &name_bone_map = anim->get_name_bone_map();
	// for (auto &name_bone : name_bone_map)
	// {
	// const std::string &name = name_bone.first;
	// const glm::mat4 &bindpose = name_bone.second->get_bindpose();
	// animation_bindpose_[name] = bindpose;
	// }
	animation_bindpose_ = anim->name_bindpose_map_;
#ifndef NDEBUG
	std::cout << "Retargeter:: bindpose size: " << animation_bindpose_.size() << std::endl;
	for (auto& name_bindpose : animation_bindpose_)
	{
		std::cout << "=======\n";
		std::cout << glm::to_string(name_bindpose.second) << std::endl;
		std::cout << "=======\n";
	}
#endif
}

void MixamoRetargeter::init_animation(Animation* anim)
{
	new_animation_->duration_ = anim->duration_;
	new_animation_->fps_ = anim->fps_;
	new_animation_->name_ = anim->name_;
	new_animation_->type_ = AnimationType::Raw;

	std::set<float> time_set;

	auto& name_bone_map = anim->get_name_bone_map();
	for (auto& name_bone : name_bone_map)
	{
		auto& bone_time_set = name_bone.second->get_time_set();
		time_set.insert(bone_time_set.begin(), bone_time_set.end());
		new_animation_->name_bone_map_[name_bone.first] = std::make_unique<Bone>();
		new_animation_->name_bone_map_[name_bone.first].get()->set_name(name_bone.first);
		new_animation_->name_bone_map_[name_bone.first].get()->set_bindpose(new_animation_bindpose_[name_bone.first]);
	}
#ifndef NDEBUG
	std::cout << "time: " << *time_set.rbegin() << std::endl;
#endif

	for (auto time : time_set)
	{
		push_new_animation(time, root_, name_bone_map, glm::mat4(1.0f), glm::mat4(1.0f));
	}
}
void MixamoRetargeter::push_new_animation(float time,
										  AnimationTempNode& node,
										  const std::map<std::string, std::unique_ptr<Bone>>& name_bone_map,
										  glm::mat4 parent_mat,
										  glm::mat4 n_parent_mat)
{
	if (auto name_bindpose = animation_bindpose_.find(node.name); name_bindpose != animation_bindpose_.end())
	{
		glm::mat4& AB = animation_bindpose_[node.name];
		glm::mat4& NAB = new_animation_bindpose_[node.name];

		if (auto bone = name_bone_map.find(node.name); bone != name_bone_map.end())
		{
			auto& bone_timeset = bone->second->get_time_set();
			glm::mat4& AR = bone->second->get_local_transform(time, 1.0);
			glm::mat4& NAO = new_animation_bindpose_offset_[node.name];
			glm::mat4& AO = animation_bindpose_offset_[node.name];

			glm::mat4 new_relative =
				glm::inverse(NAB) * glm::inverse(n_parent_mat) * parent_mat * AB * AR * AO * glm::inverse(NAO);
			auto [t, r, s] = DecomposeTransform(new_relative);
			new_relative = ComposeTransform(t, r, s);
			parent_mat = parent_mat * AB * AR;
			n_parent_mat = n_parent_mat * NAB * new_relative;

			// add node
			if (auto it = bone_timeset.find(time); it != bone_timeset.end())
			{
#ifndef NDEBUG
				std::cout << node.name << ": " << time << std::endl;
#endif
				auto bone = new_animation_->get_mutable_name_bone_map()[node.name].get();
				bone->push_position(t, time, false);
				bone->push_rotation(r, time, false);
				bone->push_scale(s, time, false);
			}
		}
		else
		{
			parent_mat = parent_mat * AB;
			n_parent_mat = n_parent_mat * NAB;
		}
	}

	// dfs
	for (auto& child : node.children)
	{
		push_new_animation(time, child, name_bone_map, parent_mat, n_parent_mat);
	}
}
std::shared_ptr<Animation> MixamoRetargeter::retarget(PoseComponent* pose_component)
{
	if (!pose_component || !pose_component->get_animation_component() ||
		!pose_component->get_animation_component()->get_animation())
	{
		return nullptr;
	}
	new_animation_ = std::make_shared<Animation>();

	// target
	auto armature_root = pose_component->get_root_entity();
	// src
	auto animation_comp = pose_component->get_animation_component();
	auto anim = animation_comp->get_mutable_animation();

#ifndef NDEBUG
	std::cout << "anim type: " << anim->type_ << std::endl;
#endif

	init_animation_bindpose(animation_comp->get_animation());
	init_animation_temp_node(armature_root, root_, nullptr);

	init_animation(anim);

	return new_animation_;
}
}	 // namespace anim