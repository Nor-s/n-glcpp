#ifndef ANIM_RESOURCES_MODEL_H
#define ANIM_RESOURCES_MODEL_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include <filesystem>
#include <unordered_map>
#include <string>
#include "shared_resources.h"

#include "../entity/components/transform_component.h"

#include "../graphics/mesh.h"
#include <memory.h>
#include "../util/log.h"

namespace anim
{
class Mesh;
// can't load "../"
unsigned int TextureFromFile(const char* path, const std::filesystem::path& directory, const aiScene* scene);
void LoadMemory(const aiTexture* texture, unsigned int* id);
struct BoneInfo
{
	/*id is index in finalBoneMatrices*/
	int id;

	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;

	const glm::mat4& get_offset() const
	{
		return offset;
	}
	const int& get_id() const
	{
		return id;
	}
};
struct ModelNode
{
	glm::mat4 relative_transformation{};
	std::string name{};
	bool has_bone{false};
	std::vector<std::shared_ptr<ModelNode>> childrens;
	std::vector<std::shared_ptr<Mesh>> meshes;
	ModelNode(const glm::mat4& initial_transform, const std::string& node_name, unsigned int num_children)
		: relative_transformation(initial_transform), name(node_name), has_bone(false)
	{
		childrens.resize(num_children);
	}
	~ModelNode()
	{
		LOG("~ModelNode: " + name);
		childrens.clear();
		meshes.clear();
	}
};
/**
 * load_model => process_node => process_mesh(mesh... vertices, indices, textures...)
 */
class Model
{
public:
	friend void SharedResources::convert_to_entity(std::shared_ptr<Entity>& entity,
												   std::shared_ptr<Model>& model,
												   const std::shared_ptr<ModelNode>& model_node,
												   Entity* parent_entity,
												   int child_num,
												   Entity* root_entity);

	Model(const char* path, const aiScene* scene);
	~Model();

	// void draw_armature(ArmatureNode &armature, Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
	// const glm::mat4 &world, float depth);

	const std::unordered_map<std::string, BoneInfo>& get_bone_info_map() const;
	std::unordered_map<std::string, BoneInfo>& get_mutable_bone_info_map();
	const BoneInfo* get_pointer_bone_info(const std::string& bone_name) const;
	int& get_mutable_bone_count();
	const std::shared_ptr<ModelNode>& get_root_node() const;
	// ArmatureNode *get_mutable_armature();

	std::shared_ptr<ModelNode>& get_mutable_root_node();
	void get_ai_node_for_anim(aiNode* ai_node, ModelNode* model_node, aiNode* parent_ai_node);
	void get_ai_root_node_for_anim(aiNode* ai_node);
	const std::string& get_name() const;
	const std::unordered_map<std::string, std::string>& get_mesh_name_node_name_map() const
	{
		return mesh_name_node_name_map_;
	}

private:
	void load_model(const char* path, const aiScene* scene);
	/**
	 * @brief 루트노드를 처음에 입력받아, 자식노드들을 순회하면서 메쉬들을 찾음
	 *
	 * @param node
	 * @param scene
	 */
	void process_node(std::shared_ptr<ModelNode>& model_node, aiNode* ai_node, const aiScene* scene);
	/**
	 * @brief 모든 vertex 데이터를 얻고, mesh의 indices를 얻고, 연관된 material(texture) 데이터를 얻음
	 *
	 * @param mesh
	 * @param scene
	 * @return Mesh
	 */
	std::shared_ptr<Mesh> process_mesh(aiMesh* mesh, const aiScene* scene);
	/**
	 * @brief bone_info_map_에 bone 데이터를 넣고, vertex에 영향을 주는 bone과 그의 영향(가중치)을 집어넣음
	 *
	 * @param mesh
	 * @param scene
	 * @param vertices
	 */
	void process_bone(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices);

	// void process_armature(const std::shared_ptr<ModelNode> &model_node, std::shared_ptr<ArmatureNode> &armature,
	// ArmatureNode *parent_armature, int child_num);

	std::vector<Texture> load_material_textures(aiMaterial* mat,
												const aiScene* scene,
												aiTextureType type,
												std::string typeName);

private:
	std::shared_ptr<ModelNode> root_node_;
	std::unordered_map<std::string, BoneInfo> bone_info_map_;
	std::unordered_map<std::string, std::string> mesh_name_node_name_map_;
	std::filesystem::path directory_{};
	std::vector<Texture> textures_loaded_;
	std::string name_{};
	int bone_count_ = 0;
	int node_count_ = 0;
};
}	 // namespace anim

#endif