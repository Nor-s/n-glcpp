#ifndef UI_IMGUI_MENU_BAR_LAYER_H
#define UI_IMGUI_MENU_BAR_LAYER_H

#include "timeline_layer.h"
#include "hierarchy_layer.h"
#include "component_layer.h"

#include <string>
#include <memory>
#include <map>

struct GLFWwindow;

class Scene;
namespace anim
{
class SharedResources;
class Entity;
}	 // namespace anim

namespace ui
{
class SceneLayer;

/**
 * @brief dock + menu bar(import, export)
 *
 */
class MainLayer
{
public:
	MainLayer();
	~MainLayer();

	void init(GLFWwindow* window);
	void begin();
	void end();
	void draw_dock(float fps);
	void draw_scene(const std::string& title, Scene* scene);
	void draw_component_layer(Scene* scene);

	void draw_hierarchy_layer(Scene* scene);
	void draw_timeline(Scene* scene);

	bool is_scene_layer_hovered(const std::string& title);

	const UiContext& get_context() const;

private:
	void init_bookmark();
	void shutdown();
	void draw_menu_bar(float fps);
	void draw_python_modal(bool& is_open);
	// https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/
	std::map<std::string, std::unique_ptr<SceneLayer>> scene_layer_map_;
	HierarchyLayer hierarchy_layer_{};
	ComponentLayer component_layer_{};
	TimelineLayer timeline_layer_{};
	UiContext context_{};
};
}	 // namespace ui

#endif