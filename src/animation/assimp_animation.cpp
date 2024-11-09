#include "assimp_animation.h"

#include "log.h"
#include "../util/utility.h"
#include "morph_target.h"

namespace fs = std::filesystem;

namespace anim
{
AssimpAnimation::AssimpAnimation(const aiAnimation* animation, const aiScene* scene, const char* path)
{
	init_animation(animation, scene, path);
}

AssimpAnimation::~AssimpAnimation() = default;

void AssimpAnimation::init_animation(const aiAnimation* animation, const aiScene* scene, const char* path)
{
	type_ = AnimationType::Assimp;

	path_ = std::string(path);
	fs::path anim_path = fs::u8path(path_);
	name_ = anim_path.filename().string();
	duration_ = animation->mDuration;
	fps_ = animation->mTicksPerSecond;
	process_bones(animation, scene->mRootNode);
	process_bindpose(scene->mRootNode);
}
void AssimpAnimation::process_bones(const aiAnimation* animation, const aiNode* root_node)
{
	int size = animation->mNumChannels;

	// reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i];
		std::string bone_name = channel->mNodeName.C_Str();

		const aiNode* node = root_node->FindNode(channel->mNodeName);
		if (node)
		{
			name_bone_map_[bone_name] =
				std::make_unique<Bone>(bone_name, channel, glm::inverse(AiMatToGlmMat(node->mTransformation)));
		}
	}
}
void AssimpAnimation::process_bindpose(const aiNode* node)
{
	if (node)
	{
		std::string name(node->mName.C_Str());
		auto bindpose = AiMatToGlmMat(node->mTransformation);
		auto children_size = node->mNumChildren;
		name_bindpose_map_[name] = bindpose;
		for (unsigned int i = 0; i < children_size; ++i)
		{
			process_bindpose(node->mChildren[i]);
		}
	}
}

}	 // namespace anim
