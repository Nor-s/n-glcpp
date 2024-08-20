#ifndef ANIM_ANIMATION_ANIMATION_H
#define ANIM_ANIMATION_ANIMATION_H

#include "bone.h"
// #include "retargeter.h"

#include <assimp/scene.h>

#include <string>
#include <map>
#include <filesystem>
#include <iostream>
#include <memory>

namespace anim
{
enum AnimationType
{
	None,
	Assimp,
	Json,
	Raw
};

class Animation
{
	friend class MixamoRetargeter;

public:
	Animation() = default;
	Animation(std::string_view file_path);
	virtual ~Animation() = default;
	virtual glm::mat4 get_bone_local_transform(const std::string_view name,
											   const float time,
											   const float factor,
											   bool bIsRemoveTranslation);
	float get_fps();
	float get_duration();
	float get_current_duration();

	const std::string& get_name() const;
	const char* get_path() const;
	const std::map<std::string, std::unique_ptr<Bone>>& get_name_bone_map() const;
	std::map<std::string, std::unique_ptr<Bone>>& get_mutable_name_bone_map();
	const AnimationType& get_type() const;

	virtual void reload();
	void get_ai_animation(aiAnimation* ai_anim, const aiNode* ai_root_node, float factor = 1.0, bool is_linear = true);
	void set_id(int id);
	const int get_id() const;
	void insert_and_update_keyframe(const std::string& name, const glm::mat4& transform, float time);
	void remove_keyframe(const std::string& name, float time);
	void update_keyframe(const std::string& name, const glm::mat4& transform, float time);

private:
	Bone* find_bone(const std::string_view name);

protected:
	float duration_{0.0f};
	int fps_{0};
	std::string name_{};
	std::map<std::string, std::unique_ptr<Bone>> name_bone_map_{};
	std::map<std::string, glm::mat4> name_bindpose_map_{};
	AnimationType type_{};
	std::string path_{};
	int id_{-1};
};

}	 // namespace anim

#endif