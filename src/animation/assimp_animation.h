#ifndef ANIM_ANIMATION_ASSIMP_ANIMATION_H
#define ANIM_ANIMATION_ASSIMP_ANIMATION_H

#include "animation.h"
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <functional>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <set>

namespace anim
{
class AssimpAnimation : public Animation
{
public:
	AssimpAnimation() = delete;

	AssimpAnimation(const aiAnimation* animation, const aiScene* scene, const char* path);

	~AssimpAnimation();

private:
	void init_animation(const aiAnimation* animation, const aiScene* scene, const char* path);
	void process_bones(const aiAnimation* animation, const aiNode* root_node);
	void process_bindpose(const aiNode* node);
	void process_morph_target(const aiScene* scene);
};

}	 // namespace anim

#endif