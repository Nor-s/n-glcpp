#ifndef ANIM_RESOURCES_SHARED_RESOURCES_H
#define ANIM_RESOURCES_SHARED_RESOURCES_H

#include "../graphics/mesh.h"
#include <memory>
#include <map>

namespace anim
{

    class Animator;
    class Shader;
    class Animation;
    class Entity;
    class Model;
    struct ModelNode;
    class SharedResources
    {
    public:
        SharedResources();
        ~SharedResources();
        Animator *get_mutable_animator();
        std::shared_ptr<Shader> get_mutable_shader(const std::string &name);
        void import(const char *path);
        void add_entity(std::shared_ptr<Model> &model);
        void add_animations(const std::vector<std::shared_ptr<Animation>> &animations);
        void add_shader(const std::string &name, const char *vs_path, const char *fs_path);
        void convert_to_entity(std::shared_ptr<Entity> &entity, std::shared_ptr<Model> &model, const std::shared_ptr<ModelNode> &model_node, Entity *parent_entity);
        void update();
        void set_ubo_projection(const glm::mat4 &projection);
        void set_ubo_view(const glm::mat4 &view);
        void set_dt(float dt);

    private:
        void init_animator();
        void init_shader();
        std::unique_ptr<Animator> animator_;
        std::vector<std::shared_ptr<Animation>> animations_;
        std::map<std::string, std::shared_ptr<Shader>> shaders_;
        std::vector<std::shared_ptr<Entity>> single_entity_list_;
        std::shared_ptr<Entity> root_entity_;
        std::unique_ptr<Mesh> bone_;
        float dt_ = 0.0f;
        unsigned int matrices_UBO_;
    };

}
#endif