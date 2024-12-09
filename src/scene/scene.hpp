#ifndef SRC_SCENE_SCENE_H
#define SRC_SCENE_SCENE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <memory>
#include <entity/components/component.h>
#include <../resources/shared_resources.h>
#include <../entity/entity.h>
#include <../entity/components/pose_component.h>
#include <../entity/components/renderable/armature_component.h>
#include "../entity/components/renderable/polygon_component.h"

namespace glcpp
{
class Camera;
}
namespace anim
{
class Framebuffer;
}

struct DebugInfo
{
	inline static float default_scale = 10.0f;
	DebugInfo(const glm::mat4& a_transform,
			  anim::PolygonType type = anim::PolygonType::BI_PYRAMID,
			  uint32_t color = 0xff0000ff)
		: transform(a_transform), type(type), color(color)
	{
		transform *= glm::scale(glm::mat4(1.0f), glm::vec3(default_scale));
	}
	anim::PolygonType type;
	glm::mat4 transform;
	uint32_t color = 0xff0000ff;
};

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;
	virtual void init_framebuffer(uint32_t width, uint32_t height) = 0;
	virtual void pre_draw() = 0;
	virtual void draw() = 0;
	virtual void picking(int x, int y, bool is_only_bone) = 0;
	virtual anim::Entity* get_mutable_selected_entity()
	{
		return selected_entity_;
	}
	virtual std::shared_ptr<anim::Framebuffer> get_mutable_framebuffer()
	{
		return framebuffer_;
	}
	virtual std::shared_ptr<anim::SharedResources> get_mutable_shared_resources()
	{
		return resources_;
	}
	virtual std::shared_ptr<anim::SharedResources>& get_mutable_ref_shared_resources()
	{
		return resources_;
	}
	virtual std::shared_ptr<glcpp::Camera>& get_mutable_ref_camera()
	{
		return camera_;
	}
	virtual void set_size(uint32_t width, uint32_t height)
	{
		if (width > 0)
			width_ = width;
		if (height > 0)
			height_ = height;
	}
	virtual void set_delta_time(float dt)
	{
		delta_time_ = dt;
		resources_->set_dt(delta_time_);
	}
	void set_selected_entity(anim::Entity* entity)
	{
		if (selected_entity_)
		{
			selected_entity_->set_is_selected(false);
		}
		if (entity)
		{
			entity->set_is_selected(true);
		}

		selected_entity_ = entity;
	}
	void set_selected_entity(int id)
	{
		if (id == -1 && selected_entity_)
		{
			id = selected_entity_->get_mutable_root()->get_id();
		}
		set_selected_entity(resources_->get_entity(id));
	}
	void draw_debug(const DebugInfo& debug_info)
	{
		debug_infos_.push_back(debug_info);
	}

protected:
	void draw_debug()
	{
		anim::Entity* debug_entity = resources_->get_debug_entity();
		for (auto& debug_info : debug_infos_)
		{
			debug_entity->get_component<anim::PolygonComponent>()
				->set_type(debug_info.type)
				->set_color(debug_info.color);
			debug_entity->set_local(debug_info.transform);
			debug_entity->update();
		}
		debug_infos_.clear();
	}

protected:
	anim::Entity* selected_entity_{nullptr};
	std::shared_ptr<anim::SharedResources> resources_;
	std::shared_ptr<anim::Framebuffer> framebuffer_;
	std::shared_ptr<glcpp::Camera> camera_;
	std::vector<DebugInfo> debug_infos_;
	float delta_time_ = 0.0f;
	uint32_t width_ = 800;
	uint32_t height_ = 600;
};

#endif