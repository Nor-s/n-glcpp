#ifndef ANIM_ENTITY_COMPONENT_POLYGON_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_POLYGON_COMPONENT_H

#include "../component.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

namespace anim
{
class Shader;
class Mesh;
class PoseComponent;
class Entity;

enum class PolygonType
{
	BI_PYRAMID
};

class PolygonComponent : public ComponentBase<PolygonComponent>
{
public:
	PolygonComponent* set_type(PolygonType type);

	PolygonComponent* set_shader(Shader* shader)
	{
		shader_ = shader;
		return this;
	}

	PolygonComponent* set_color(uint32_t color)
	{
		color = color;
		return this;
	}

	void update();

private:
	void update_polygon();

private:
	static std::unique_ptr<Mesh> bipyramid_;

private:
	uint32_t color;
	Shader* shader_;
	Mesh* polygon_;
	PolygonType type_;
};
}	 // namespace anim

#endif