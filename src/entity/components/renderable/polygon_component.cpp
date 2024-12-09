#include "polygon_component.h"
#include "../../../graphics/shader.h"
#include "../../../graphics/mesh.h"
#include "../../entity.h"
#include "../graphics/opengl/gl_mesh.h"

namespace anim
{
std::unique_ptr<Mesh> PolygonComponent::bipyramid_;
PolygonComponent* PolygonComponent::set_type(PolygonType type)
{
	type_ = type;
	return this;
}
void PolygonComponent::update()
{
	update_polygon();
	shader_->use();
	shader_->set_mat4("model", get_owner()->get_world_transformation());
	shader_->set_uint("color", color);
	polygon_->draw(*shader_);
}

void PolygonComponent::update_polygon()
{
	switch (type_)
	{
		case PolygonType::BI_PYRAMID:
			if (bipyramid_ == nullptr)
			{
				bipyramid_ = gl::CreateBiPyramid();
			}
			polygon_ = bipyramid_.get();
			break;
	}
}

}	 // namespace anim