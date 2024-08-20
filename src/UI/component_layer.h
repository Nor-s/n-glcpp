#ifndef UI_IMGUI_COMPONENT_LAYER_H
#define UI_IMGUI_COMPONENT_LAYER_H

#include "ui_context.h"

class Scene;
namespace anim
{
	class SharedResources;
	class Entity;
	class TransformComponent;
	class AnimationComponent;
	class MeshComponent;
	class RealTimeComponent;
}

namespace ui
{
	class ComponentLayer
	{
	public:
		ComponentLayer();
		~ComponentLayer();
		void draw(ComponentContext& context, Scene* scene);

	private:
		void draw_animation(ComponentContext& context, const anim::SharedResources* shared_resource, const anim::Entity* entity, const anim::AnimationComponent* animation);
		void draw_transform(anim::Entity* entity);
		void draw_transform_reset_button(anim::TransformComponent& transform);
		void draw_realtime_component(anim::RealTimeComponent& realtime_component);
		void draw_mesh(anim::MeshComponent* mesh);
		void draw_manupulate_component_button(anim::Entity* entity);
	};
}

#endif
