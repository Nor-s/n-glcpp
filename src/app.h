#ifndef SRC_PIXEL3D_H
#define SRC_PIXEL3D_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "glcpp/application.hpp"

struct GLFWwindow;
class Scene;
namespace anim
{
class SharedResources;
class EventHistoryQueue;
class Event;
}	 // namespace anim

namespace ui
{
class MainLayer;
}

class App : public glcpp::Application<App>
{
	template <typename T>
	friend class glcpp::Application;

public:
	~App();
	void init(uint32_t width, uint32_t height, const std::string& title) override;

	void loop() override;
	void update();
	void update_window();
	void update_time();

	void pre_draw();
	void draw_scene();
	void post_draw();
	void process_input(GLFWwindow* window);

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
	static void mouse_btn_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	void history_push(std::unique_ptr<anim::Event> event);
	Scene* get_current_scene() const;

private:
	// mouse event
	bool is_pressed_ = false;
	bool is_pressed_scroll_ = false;
	glm::vec2 prev_mouse_{-1.0f, -1.0f}, cur_mouse_{-1.0f, -1.0f};
	uint32_t current_scene_idx_ = 0u;
	std::unique_ptr<ui::MainLayer> ui_;
	// timing
	float start_time_ = 0.0f;
	float delta_frame_ = 0.0f;
	float last_frame_ = 0.0f;
	float fps_ = 0.0f;
	int frames_ = 0;
	bool is_manipulated_ = false;
	bool is_dialog_open_ = false;
	std::vector<std::shared_ptr<Scene>> scenes_;

private:
	App();
	void init_window(uint32_t width, uint32_t height, const std::string& title);
	void init_callback();
	void init_ui();
	void init_shared_resources();
	void init_scene(uint32_t width, uint32_t height);
	void post_update();
	void process_menu_context();
	void process_timeline_context();
	void process_scene_context();
	void process_component_context();
	void process_python_context();

	void import_model_or_animation(const char* const path);

	std::shared_ptr<anim::SharedResources> shared_resources_;
	std::unique_ptr<anim::EventHistoryQueue> history_;
};

#endif
