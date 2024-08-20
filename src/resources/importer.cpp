#include "importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include "../entity/entity.h"
#include "../animation/json_animation.h"
#include "../animation/assimp_animation.h"
#include "model.h"
#include "../util/log.h"

namespace fs = std::filesystem;

namespace anim
{
Importer::Importer()
	: assimp_flag_(aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality |
				   aiProcess_LimitBoneWeights | aiProcess_RemoveRedundantMaterials | aiProcess_SplitLargeMeshes |
				   aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType | aiProcess_FindDegenerates |
				   aiProcess_FindInvalidData | aiProcess_FindInstances | aiProcess_ValidateDataStructure |
				   aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph)
{
}

std::pair<std::shared_ptr<Model>, std::vector<std::shared_ptr<Animation>>> Importer::read_file(const char* path)
{
	path_ = std::string(path);
	std::vector<std::shared_ptr<Animation>> animations;
	std::shared_ptr<Model> model;
	fs::path fs_path = fs::u8path(path_);

	if (fs_path.extension() == ".json")
	{
		std::shared_ptr<JsonAnimation> anim = std::make_shared<JsonAnimation>(path_.c_str());
		if (anim && anim->get_type() == AnimationType::Json)
		{
			animations.emplace_back(anim);
		}
		return std::make_pair(nullptr, animations);
	}
	try
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, 128);
		const aiScene* scene = importer.ReadFile(path_.c_str(), assimp_flag_);

		model = import_model(scene);
		animations = import_animation(scene);
		if (model == nullptr && animations.size() == 0)
		{
			LOG("ERROR::IMPORTER::NULL " + std::string(importer.GetErrorString()));
		}
	}
	catch (std::exception& e)
	{
		LOG("ERROR::IMPORTER: " + std::string(e.what()));
	}
	return std::make_pair(model, animations);
}
std::shared_ptr<Model> Importer::import_model(const aiScene* scene)
{
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		return nullptr;
	}
	return std::make_shared<Model>(path_.c_str(), scene);
}
std::vector<std::shared_ptr<Animation>> Importer::import_animation(const aiScene* scene)
{
	std::vector<std::shared_ptr<Animation>> animations;
	if (scene && scene->mRootNode)
	{
		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			auto animation = scene->mAnimations[i];
			animations.push_back(std::make_shared<AssimpAnimation>(animation, scene, path_.c_str()));
		}
	}
	return animations;
}
}	 // namespace anim
