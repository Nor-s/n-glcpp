#ifndef ANIM_ANIMATION_RETARGETER_H
#define ANIM_ANIMATION_RETARGETER_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace anim
{
/**
 * @brief SMPL to MIXAMO
 */
class MixamoRetargeter
{
	// NAR = (NAB)-1 * NParent-1 * Parent * AB  * AR * AOffset * (NAOffset)-1
	struct AnimationTempNode
	{
		bool exist_bone = false;
		// animation bindpose
		glm::mat4 AB;
		// new animation bindpose
		glm::vec3 NAB_trans = glm::vec3(0.0f);
		glm::quat NAB_rotat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 NAB_scale = glm::vec3(1.0f);
		// animagion bindpose world transform
		glm::mat4 n_world_transform = glm::mat4(1.0f);
		glm::mat4 world_transform = glm::mat4(1.0f);
		std::string name;
		std::vector<AnimationTempNode> children;
	};

public:
	MixamoRetargeter() = default;
	~MixamoRetargeter() = default;
	std::shared_ptr<class Animation> retarget(class PoseComponent* pose_component);

private:
	void init_animation_temp_node(class Entity* entity, AnimationTempNode& anim_node, AnimationTempNode* parent_node);
	void init_animation_bindpose(const class Animation* anim);
	void init_animation(class Animation* anim);
	void push_new_animation(float time,
							AnimationTempNode& node,
							const std::map<std::string, std::unique_ptr<class Bone>>& name_bone_map,
							glm::mat4 parent_mat,
							glm::mat4 n_parent_mat);

private:
	AnimationTempNode root_;
	std::map<std::string, glm::mat4> animation_bindpose_;
	std::map<std::string, glm::mat4> new_animation_bindpose_;
	std::map<std::string, glm::mat4> animation_bindpose_offset_;
	std::map<std::string, glm::mat4> new_animation_bindpose_offset_;

	std::shared_ptr<class Animation> new_animation_;
};
}	 // namespace anim

#endif