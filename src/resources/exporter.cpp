#include "exporter.h"
#include "framebuffer.h"
#include "model.h"
#include "../util/utility.h"
#include "../entity/entity.h"
#include "../entity/components/renderable/armature_component.h"
#include "../entity/components/pose_component.h"
#include "../entity/components/transform_component.h"
#include "../entity/components/animation_component.h"
#include "../animation/animation.h"
#include "importer.h"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <json/json.h>
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>

#include <fstream>
#include <filesystem>
#include <set>
#include <map>

namespace anim
{

Json::Value get_quat_json(const glm::quat& r)
{
	Json::Value ret_json;
	ret_json["w"] = r.w;
	ret_json["x"] = r.x;
	ret_json["y"] = r.y;
	ret_json["z"] = r.z;
	return ret_json;
}
Json::Value get_vec_json(const glm::vec3& p)
{
	Json::Value ret_json;
	ret_json["x"] = p.x;
	ret_json["y"] = p.y;
	ret_json["z"] = p.z;
	return ret_json;
}
void Exporter::to_png(Framebuffer* framebuffer, const char* save_path)
{
	auto format = framebuffer->get_color_format();
	auto width = framebuffer->get_width();
	auto height = framebuffer->get_height();
	auto FBO = framebuffer->get_fbo();

	stbi_flip_vertically_on_write(true);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, width, height);
	size_t size = static_cast<size_t>(width * height);
	GLubyte* pixels = (GLubyte*) malloc(size * sizeof(GLubyte) * 4u);
	glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);

	if (format == GL_RGBA)
		stbi_write_png(save_path, width, height, 4, pixels, 0);
	else if (format == GL_RGB)
		stbi_write_png(save_path, width, height, 3, pixels, 0);

	free(pixels);
}
void Exporter::to_json(Entity* model, const char* save_path)
{
	if (model->get_component<ArmatureComponent>() == nullptr)
	{
		return;
	}
	Json::Value result_json;
	result_json["node"] = dfs(model, "");
	result_json["name"] = "model";
	std::ofstream json_stream(save_path, std::ofstream::binary);
	json_stream << result_json.toStyledString();
	if (json_stream.is_open())
	{
		json_stream.close();
	}
}
std::string Exporter::to_json(Entity* model)
{
	if (model->get_component<ArmatureComponent>() == nullptr)
	{
		return "";
	}
	Json::Value result_json;
	result_json["node"] = dfs(model, "");
	result_json["name"] = "model";
#ifndef NDEBUG
	std::ofstream json_stream("./model.json", std::ofstream::binary);
	json_stream << result_json.toStyledString();
	if (json_stream.is_open())
	{
		json_stream.close();
	}
#endif
	return std::string(result_json.toStyledString().c_str());
}
Json::Value Exporter::dfs(Entity* node, const std::string& parent_name)
{
	Json::Value node_json;
	auto armature = node->get_component<ArmatureComponent>();
	auto& transform = armature->get_bindpose();
	const auto& [t, r, s] = DecomposeTransform(transform);
	node_json["name"] = node->get_name();
	node_json["position"] = get_vec_json(t);
	node_json["rotation"] = get_quat_json(r);
	node_json["scale"] = get_vec_json(s);
	node_json["parent_name"] = parent_name;
	node_json["child"] = Json::arrayValue;
	for (auto& sub_node : node->get_mutable_children())
	{
		node_json["child"].append(dfs(sub_node.get(), node->get_name()));
	}
	return node_json;
}

// assimp export not stable
// "mixamorig:LeftForeArm"
void Exporter::to_glft2(Entity* entity, const char* save_path, const char* model_path)
{
	if (!entity->get_component<AnimationComponent>())
	{
		return;
	}

	Assimp::Importer importer;
	auto assimp_flag_ = aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality |
						aiProcess_LimitBoneWeights | aiProcess_RemoveRedundantMaterials | aiProcess_SplitLargeMeshes |
						aiProcess_Triangulate | aiProcess_GenUVCoords |
						// aiProcess_GlobalScale |
						aiProcess_SortByPType | aiProcess_FindDegenerates | aiProcess_FindInvalidData |
						aiProcess_FindInstances | aiProcess_ValidateDataStructure |
						// aiProcess_OptimizeGraph|
						aiProcess_OptimizeMeshes;

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, 128);
	// importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 100.0);

	aiScene* scene = const_cast<aiScene*>(importer.ReadFile(model_path, assimp_flag_));
	// //       // aiScene *scene = new aiScene();
	// //     // scene->mRootNode = new aiNode();
	// //   // to_ai_node(scene->mRootNode, root_entity);

	auto animation_comp = entity->get_component<AnimationComponent>();
	auto animation = animation_comp->get_mutable_animation();
	// delete scene->mAnimations[0];
	scene->mAnimations = new aiAnimation*[1];
	scene->mAnimations[0] = new aiAnimation();
	scene->mNumAnimations = 1;

	animation->get_ai_animation(scene->mAnimations[0], scene->mRootNode, animation_comp->get_ticks_per_second_factor(),
								is_linear_);

	Assimp::Exporter exporter;
	auto save = std::filesystem::u8path(save_path);
	std::string ext = save.extension().string();
	std::string format = ext;
	std::string output_path{save_path};
	if (ext.size() < 3)
	{
		output_path += ".fbx";
	}

	if (ext.size() > 2 && ext[1] == 'g')
	{
		ext = "gltf2";
		format = ext;
	}
	else
	{
		ext = "fbx";
		format = "fbx";
	}

	LOG(std::string(save_path) + ": " + ext);
	if (exporter.Export(scene, format, output_path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices) !=
		AI_SUCCESS)
	{
		std::cerr << exporter.GetErrorString() << std::endl;
	}
}

void Exporter::to_ai_node(aiNode* ai_node, Entity* entity, aiNode* parent_ai_node)
{
	auto& childrens = entity->get_mutable_children();
	auto armature = entity->get_component<ArmatureComponent>();
	glm::mat4 transform = entity->get_local();
	if (armature)
	{
		transform = armature->get_bindpose();
	}

	ai_node->mName = aiString(entity->get_name().c_str());
	ai_node->mTransformation = GlmMatToAiMat(transform);
	ai_node->mParent = parent_ai_node;
	ai_node->mNumChildren = childrens.size();
	ai_node->mChildren = new aiNode*[ai_node->mNumChildren];

	for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
	{
		ai_node->mChildren[i] = new aiNode();
		to_ai_node(ai_node->mChildren[i], childrens[i].get(), ai_node);
	}
}
void init_name(Entity* entity, Json::Value& name_list)
{
	name_list.append(entity->get_name());
	for (auto& child : entity->get_mutable_children())
	{
		init_name(child.get(), name_list);
	}
}
void init_json_pose(Json::Value& world_value, Json::Value& local_value, Entity* entity, const glm::vec3& root_pos)
{
	auto transformation = entity->get_component<ArmatureComponent>()->get_model();
	TransformComponent transform(transformation);
	glm::vec3 pos = transform.get_translation() - root_pos;
	world_value[entity->get_name()] = get_vec_json(pos);

	auto [t, r, s] = DecomposeTransform(entity->get_local());
	local_value[entity->get_name()] = get_quat_json(r);

	for (auto& child : entity->get_mutable_children())
	{
		init_json_pose(world_value, local_value, child.get(), root_pos);
	}
}
void to_json_all_animation_data(const char* save_path, Entity* entity, SharedResources* resources)
{
	if (!(entity && entity->get_mutable_root() && entity->get_mutable_root()->get_component<AnimationComponent>()))
	{
		return;
	}
	auto root = entity->get_mutable_root();
	auto animation_comp = root->get_component<AnimationComponent>();
	auto pose_comp = root->get_component<PoseComponent>();
	auto pose_root_entity = pose_comp->get_root_entity();
	auto size = resources->get_animations().size();
	auto animator = resources->get_mutable_animator();
	float before_time = animator->get_current_time();
	bool before_rootmotion = animator->mIsRootMotion;
	animator->mIsRootMotion = true;
	Json::Value ret;
	ret["pose"] = Json::arrayValue;
	ret["name_list"] = Json::arrayValue;
	init_name(pose_root_entity, ret["name_list"]);
	int pose_size = 0;
	for (int i = 0; i < size; i++)
	{
		auto animation = resources->get_mutable_animation(i);
		if (animation->get_id() != animation_comp->get_animation()->get_id())
			animation_comp->set_animation(animation);
		float duration = floor(animation->get_current_duration() * animation_comp->get_ticks_per_second_factor());
		for (float time = 0.0f; time <= duration; time += 1.0f)
		{
			Json::Value pose;
			animator->set_current_time(time);
			pose_comp->update();
			TransformComponent hips(pose_root_entity->get_component<ArmatureComponent>()->get_model());
			init_json_pose(pose["world"], pose["rotation"], pose_root_entity, hips.get_translation());
			ret["pose"].append(pose);
			pose_size++;
		}
	}
	animator->set_current_time(before_time);
	animator->mIsRootMotion = before_rootmotion;

	ret["pose_size"] = pose_size;

	try
	{
		std::ofstream json_stream(save_path, std::ofstream::binary);
		json_stream << ret.toStyledString();
		if (json_stream.is_open())
		{
			json_stream.close();
		}
	}
	catch (std::exception& e)
	{
		LOG(e.what());
	}
}

}	 // namespace anim