#ifndef ANIM_ENTITY_ENTITY_H
#define ANIM_ENTITY_ENTITY_H

#include <vector>
#include <memory>
#include <string>

#include "components/component.h"
#include "components/transform_component.h"
#include "../util/log.h"
namespace anim
{

    class Entity
    {
    public:
        bool is_deactivate_ = false;
        Entity() = default;
        Entity(const std::string &name, int id, Entity *parent = nullptr);
        template <class T>
        T *get_component();
        template <class T>
        T *add_component();
		void pre_update();
        void update();
		void post_update();
		Entity* find(const std::string& name);
		Entity* find(Entity* entity, const std::string& name);

        void set_name(const std::string &name);
        void add_children(std::shared_ptr<Entity> &children);
        void set_sub_child_num(int num);
        const std::string &get_name()
        {
            return name_;
        }
        const glm::mat4 &get_local() const
        {
            return local_;
        }
        void set_local(const glm::mat4 &local)
        {
            local_ = local;
        }
        std::vector<std::shared_ptr<Entity>> &get_mutable_children()
        {
            return children_;
        }
        void set_world_transformation(const glm::mat4 &world)
        {
            world_ = world;
        }
        const glm::mat4 &get_world_transformation() const
        {
            return world_;
        }
        void set_parent(Entity *entity)
        {
            parent_ = entity;
        }
        void set_root(Entity *entity)
        {
            root_ = entity;
        }
        Entity *get_mutable_parent()
        {
            return parent_;
        }
        Entity *get_mutable_root()
        {
            return root_;
        }
        int get_id()
        {
            return id_;
        }
        void set_is_selected(bool is_selected)
        {
            is_selected_ = is_selected;
        }
        bool is_selected_{false};

    private:
        std::string name_{};
        Entity *parent_ = nullptr;
        Entity *root_ = nullptr;
        std::vector<std::shared_ptr<Component>> components_{};
        std::vector<std::shared_ptr<Entity>> children_{};
        glm::mat4 world_{1.0f};
        glm::mat4 local_{1.0f};
        int id_{-1};
    };

    template <class T>
    T *Entity::get_component()
    {
        for (auto &component : components_)
        {
            if (component->get_type() == T::type)
            {
                return static_cast<T *>(component.get());
            }
        }
        return nullptr;
    }
    template <class T>
    T *Entity::add_component()
    {
        auto cmp = get_component<T>();
        if (cmp)
        {
            return cmp;
        }
        components_.push_back(std::make_shared<T>());
        return static_cast<T *>(components_.back().get());
    }
}

#endif