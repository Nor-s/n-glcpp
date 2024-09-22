#ifndef ANIM_RESOURCES_SHARED_RESOURCES_H
#define ANIM_RESOURCES_SHARED_RESOURCES_H

#include "../graphics/mesh.h"
#include <memory>
#include <map>
#include <unordered_map>

namespace anim
{

class Animator;
class Shader;
class Animation;
class Entity;
class Model;
struct ModelNode;
class PostProcessing;
class SharedResources
{
public:
	SharedResources();
	~SharedResources();
	Animator* get_mutable_animator();
	std::shared_ptr<Shader> get_mutable_shader(const std::string& name);
	void import(const char* path, float scale = 1.0f);
	void export_animation(Entity* entity, const char* path, bool is_linear);
	std::shared_ptr<Entity> add_entity(std::shared_ptr<Model>& model, const char* path);
	void add_animations(const std::vector<std::shared_ptr<Animation>>& animations);
	void add_animation(std::shared_ptr<Animation> animation);
	void add_shader(const std::string& name, const char* vs_path, const char* fs_path);
	void convert_to_entity(std::shared_ptr<Entity>& entity,
						   std::shared_ptr<Model>& model,
						   const std::shared_ptr<ModelNode>& model_node,
						   Entity* parent_entity,
						   int child_num,
						   Entity* root_entity);
	void update();
	void set_ubo_projection(const glm::mat4& projection);
	void set_ubo_view(const glm::mat4& view);
	void set_dt(float dt);
	std::shared_ptr<Entity>& get_mutable_entities();
	const std::vector<std::shared_ptr<Animation>>& get_animations() const;
	Animation* get_mutable_animation(int id);

	Entity* get_entity(int id);

	std::unique_ptr<PostProcessing> mPostProcessing;

private:
	void init_animator();
	void init_shader();
	std::unique_ptr<Animator> animator_;
	std::vector<std::shared_ptr<Animation>> animations_;
	std::map<std::string, std::shared_ptr<Shader>> shaders_;
	std::unordered_map<int, std::string> model_path_;
	std::vector<std::shared_ptr<Entity>> single_entity_list_;
	std::shared_ptr<Entity> root_entity_;
	std::unique_ptr<Mesh> bone_;
	float dt_ = 0.0f;
	unsigned int matrices_UBO_;
};

}	 // namespace anim
#endif