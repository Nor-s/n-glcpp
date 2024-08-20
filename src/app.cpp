#include "app.h"

#include <stb/stb_image.h>
#include "glcpp/camera.h"
#include "glcpp/window.h"
#include "UI/main_layer.h"
#include "scene/scene.hpp"
#include "scene/main_scene.h"
#include "resources/shared_resources.h"
#include "resources/exporter.h"
#include "entity/entity.h"
#include "entity/components/component.h"
#include "entity/components/animation_component.h"
#include "entity/components/renderable/armature_component.h"
#include "animation/animator.h"
#include "animation/retargeter.h"
#include "util/log.h"
#include "UI/ui_context.h"
#include "event/event_history.h"
#include "resources/exporter.h"

#include "cpython/py_manager.h"
#include <pybind11/embed.h>

namespace fs = std::filesystem;

#ifndef NDEBUG

void error_callback(int code, const char* description)
{
	printf("error %d: %s\n", code, description);
}
#endif

App::App()
{
}

App::~App()
{
	scenes_.clear();
	ui_.reset();
}

void App::init(uint32_t width, uint32_t height, const std::string& title)
{
	stbi_set_flip_vertically_on_load(true);
	init_window(width, height, title);
	init_ui();
	init_shared_resources();
	init_scene(width, height);
	history_.reset(new anim::EventHistoryQueue());
}
void App::init_window(uint32_t width, uint32_t height, const std::string& title)
{
	window_ = std::make_unique<glcpp::Window>(width, height, title);
	window_->set_factor();
	window_->set_user_pointer(this);
	init_callback();
}
void App::init_callback()
{
	window_->set_scroll_callback(scroll_callback);
	window_->set_mouse_button_callback(mouse_btn_callback);
	window_->set_cursor_pos_callback(mouse_callback);

#ifndef NDEBUG
	glfwSetErrorCallback(error_callback);
#endif
}
void App::init_ui()
{
	ui_ = std::make_unique<ui::MainLayer>();
	ui_->init(window_->get_handle());
}
void App::init_shared_resources()
{
	shared_resources_ = std::make_shared<anim::SharedResources>();
}
void App::init_scene(uint32_t width, uint32_t height)
{
	scenes_.push_back(std::make_shared<MainScene>(width, height, shared_resources_));

	import_model_or_animation("./resources/models/mannequiny.fbx");
	scenes_[current_scene_idx_]->set_selected_entity(1);
}
void App::loop()
{
	glfwSwapInterval(0);
	start_time_ = static_cast<float>(glfwGetTime());

	while (!window_->should_close())
	{
		update();

		pre_draw();
		{
			ui_->begin();

			ui_->draw_dock(fps_);

			this->draw_scene();

			ui_->draw_component_layer(scenes_[current_scene_idx_].get());

			ui_->draw_hierarchy_layer(scenes_[current_scene_idx_].get());

			ui_->draw_timeline(scenes_[current_scene_idx_].get());

			ui_->end();
		}
		post_draw();

		post_update();
	}
}
void App::update()
{
	update_window();
	update_time();
}
void App::update_window()
{
	auto size = window_->get_framebuffer_size();
	while (size.first == 0 || size.second == 0)
	{
		window_->wait_events();
		size = window_->get_framebuffer_size();
	}
	window_->set_size(size.first, size.second);
}
void App::update_time()
{
	float current_time = static_cast<float>(glfwGetTime());
	frames_++;
	if (current_time - start_time_ >= 1.0)
	{
		fps_ = static_cast<float>(frames_) / (current_time - start_time_);
		frames_ = 0;
		start_time_ = current_time;
	}
	delta_frame_ = current_time - last_frame_;
	last_frame_ = current_time;
	shared_resources_->set_dt(delta_frame_);
}
void App::post_update()
{
	process_timeline_context();
	process_menu_context();
	process_scene_context();
	process_component_context();
	process_python_context();
}
void App::process_timeline_context()
{
	auto& ui_context = ui_->get_context();
	auto& time_context = ui_context.timeline;
	auto& entity_context = ui_context.entity;
	auto animator = shared_resources_->get_mutable_animator();
	if (time_context.is_current_frame_changed)
	{
		animator->set_current_time(static_cast<float>(time_context.current_frame));
	}
	animator->set_fps(time_context.fps);
	animator->set_end_time(static_cast<float>(time_context.end_frame));

	animator->set_is_stop(time_context.is_stop);

	if (time_context.is_clicked_play)
	{
		animator->set_is_stop(false);
		animator->set_direction(false);
	}
	if (time_context.is_clicked_play_back)
	{
		animator->set_is_stop(false);
		animator->set_direction(true);
	}
	if (entity_context.is_changed_selected_entity)
	{
		scenes_[current_scene_idx_]->set_selected_entity(entity_context.selected_id);
	}
	static uint32_t count = 0u;
	is_manipulated_ = entity_context.is_manipulated;
	if (entity_context.is_changed_transform && time_context.is_recording)
	{
		auto selected_entity = scenes_[current_scene_idx_]->get_mutable_selected_entity();
		auto& before_transform = selected_entity->get_local();
		selected_entity->set_local(entity_context.new_transform);
		if (auto armature = selected_entity->get_component<anim::ArmatureComponent>(); armature)
		{
			armature->insert_and_update_bone();
			if (count == 0u)
			{
				history_->push(std::make_unique<anim::BoneChangeEvent>(
					scenes_[current_scene_idx_].get(),
					scenes_[current_scene_idx_]->get_mutable_ref_shared_resources().get(),
					selected_entity->get_id(),
					selected_entity->get_mutable_root()->get_component<anim::AnimationComponent>()->get_animation()->get_id(),
					before_transform,
					armature->get_pose()->get_animator()->get_current_time()));
			}
			count++;
		}
	}
	else
	{
		count = 0u;
	}
	if (time_context.is_delete_current_frame)
	{
		anim::LOG("delete current bone");
		auto selected_entity = scenes_[current_scene_idx_]->get_mutable_selected_entity();
		auto& before_transform = selected_entity->get_local();
		if (auto armature = selected_entity->get_component<anim::ArmatureComponent>(); armature)
		{
			armature->get_pose()->sub_current_bone(armature->get_name());
			history_->push(std::make_unique<anim::BoneChangeEvent>(
				scenes_[current_scene_idx_].get(),
				scenes_[current_scene_idx_]->get_mutable_ref_shared_resources().get(),
				selected_entity->get_id(),
				selected_entity->get_mutable_root()->get_component<anim::AnimationComponent>()->get_animation()->get_id(),
				before_transform,
				armature->get_pose()->get_animator()->get_current_time()));
		}
	}
}

void App::process_menu_context()
{
	auto& ui_context = ui_->get_context();
	auto& menu_context = ui_context.menu;
	if (menu_context.is_clicked_import_model)
	{
		shared_resources_->import(menu_context.path.c_str(), menu_context.import_scale);
	}
	if (menu_context.is_clicked_export_animation)
	{
		shared_resources_->export_animation(scenes_[current_scene_idx_]->get_mutable_selected_entity(), menu_context.path.c_str(), menu_context.is_export_linear_interpolation);
	}
	if (menu_context.is_clicked_import_dir)
	{
		for (const auto& file : std::filesystem::directory_iterator(menu_context.path))
		{
			anim::LOG(file.path().string());
			if (file.path().extension().compare(".fbx") == 0 || file.path().extension().compare(".gltf") == 0)
			{
				anim::LOG("IMPORT");
				shared_resources_->import(file.path().string().c_str(), menu_context.import_scale);
			}
		}
	}
	if (menu_context.is_clicked_export_all_data)
	{
		auto entity = scenes_[current_scene_idx_]->get_mutable_selected_entity();
		anim::to_json_all_animation_data(menu_context.path.c_str(), entity, shared_resources_.get());
	}
}

void App::process_scene_context()
{
	auto& ui_context = ui_->get_context();
	auto& scene_context = ui_context.scene;
	is_dialog_open_ = ui_context.menu.is_dialog_open;
	if (scene_context.is_picking && !is_dialog_open_)
	{
		scenes_[current_scene_idx_]->picking(scene_context.x, scene_context.y, scene_context.is_bone_picking_mode);
	}
}

void App::process_component_context()
{
	auto& ui_context = ui_->get_context();
	auto& comp_context = ui_context.component;
	if (comp_context.is_changed_animation)
	{
		auto animation_comp = scenes_[current_scene_idx_]->get_mutable_selected_entity()->get_mutable_root()->get_component<anim::AnimationComponent>();
		animation_comp->set_animation(shared_resources_->get_mutable_animation(comp_context.new_animation_idx));
	}
	if (comp_context.is_clicked_retargeting) {
		auto pose_comp = scenes_[current_scene_idx_]->get_mutable_selected_entity()->get_mutable_root()->get_component<anim::PoseComponent>();
		auto retargeter = anim::MixamoRetargeter();
		shared_resources_->add_animation(retargeter.retarget(pose_comp));
	}
}

void App::process_python_context()
{
	auto& ui_context = ui_->get_context();
	auto& py_context = ui_context.python;
	if (py_context.is_clicked_convert_btn)
	{
		auto selected_entity = scenes_[current_scene_idx_]->get_mutable_selected_entity();
		auto selected_root = (selected_entity) ? selected_entity->get_mutable_root() : nullptr;
		auto pose_comp = (selected_root) ? selected_root->get_component<anim::PoseComponent>() : nullptr;
		if (selected_entity && selected_root && pose_comp)
		{
			anim::Exporter exporter;
			std::string model_info = exporter.to_json(pose_comp->get_root_entity());
			const char* model_info_c = model_info.c_str();
			auto py = anim::PyManager::get_instance();
			py->get_mediapipe_animation(anim::MediapipeInfo{ py_context.video_path.c_str(),
									py_context.save_path.c_str(),
									model_info_c,
									py_context.min_visibility,
									py_context.is_angle_adjustment,
									py_context.model_complexity,
									py_context.min_detection_confidence,
									py_context.fps,
									&py_context.factor });
			import_model_or_animation(py_context.save_path.c_str());
		}
	}
}

void App::import_model_or_animation(const char* const path)
{
	shared_resources_->import(path, 100.0f);
}

void App::pre_draw()
{
	process_input(window_->get_handle());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_->get_width(), window_->get_height());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::draw_scene()
{
	size_t size = scenes_.size();
	for (size_t i = 0; i < size; i++)
	{
		std::string scene_name = std::string("scene") + std::to_string(i + 1);
		scenes_[i]->pre_draw();
		ui_->draw_scene(scene_name, scenes_[i].get());
		if (ui_->is_scene_layer_hovered(scene_name))
		{
			current_scene_idx_ = i;
		}
	}
}

void App::post_draw()
{
	glfwSwapBuffers(window_->get_handle());

	glfwPollEvents();
}

void App::process_input(GLFWwindow* window)
{
	if (is_dialog_open_ || !ui_->is_scene_layer_hovered("scene" + std::to_string(current_scene_idx_ + 1)) || is_manipulated_)
	{
		return;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::FORWARD, delta_frame_);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::BACKWARD, delta_frame_);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::LEFT, delta_frame_);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::RIGHT, delta_frame_);

	static bool is_pressed_z = false;
	static bool is_released_z = false;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		is_pressed_z = true;
	}
	else if (is_pressed_z && glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
	{
		is_pressed_z = false;
		is_released_z = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && is_released_z)
	{
		anim::LOG("POP::HISTORY");
		history_->pop();
		is_released_z = false;
		shared_resources_->get_mutable_animator()->set_is_stop(true);
	}
}

void App::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (app->ui_ && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1))) {
		if (app->is_pressed_ && app->scenes_.size() > app->current_scene_idx_)
		{
			app->scenes_[app->current_scene_idx_]->get_mutable_ref_camera()->process_mouse_movement((static_cast<float>(yposIn) - app->prev_mouse_.y) / 3.6f, (static_cast<float>(xposIn) - app->prev_mouse_.x) / 3.6f);
			app->prev_mouse_.x = xposIn;
			app->prev_mouse_.y = yposIn;
		}
		if (app->is_pressed_scroll_ && app->scenes_.size() > app->current_scene_idx_)
		{
			app->scenes_[app->current_scene_idx_]->get_mutable_ref_camera()->process_mouse_scroll_press((static_cast<float>(yposIn) - app->prev_mouse_.y), (static_cast<float>(xposIn) - app->prev_mouse_.x), app->delta_frame_);
			app->prev_mouse_.x = xposIn;
			app->prev_mouse_.y = yposIn;
		}
	}
	app->cur_mouse_.x = xposIn;
	app->cur_mouse_.y = yposIn;
}

void App::mouse_btn_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && app->ui_ && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1)))
	{
		app->prev_mouse_.x = app->cur_mouse_.x;
		app->prev_mouse_.y = app->cur_mouse_.y;
		app->is_pressed_ = true;
	}
	else
	{
		app->is_pressed_ = false;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS && app->ui_ && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1)))
	{
		app->prev_mouse_.x = app->cur_mouse_.x;
		app->prev_mouse_.y = app->cur_mouse_.y;
		app->is_pressed_scroll_ = true;
	}
	else
	{
		app->is_pressed_scroll_ = false;
	}
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (app->scenes_.size() > app->current_scene_idx_ && app->ui_ && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1)))
		app->scenes_[app->current_scene_idx_]->get_mutable_ref_camera()->process_mouse_scroll(yoffset);
}