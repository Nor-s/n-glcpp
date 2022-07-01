#include "pixel3D.h"

#include <stb/stb_image.h>
#include "glcpp/camera.h"
#include "glcpp/window.h"
#include "UI/main_layer.h"
#include "scene/scene.hpp"
#include "scene/main_scene.h"
#include "scene/shared_resources.h"
#include "glcpp/model.h"
#include "glcpp/entity.h"

namespace fs = std::filesystem;

Pixel3D::Pixel3D()
{
}

Pixel3D::~Pixel3D()
{
    scenes_.clear();
    ui_.reset();
}

void Pixel3D::init(uint32_t width, uint32_t height, const std::string &title)
{
    stbi_set_flip_vertically_on_load(true);
    init_window(width, height, title);
    init_ui();
    init_shared_resources();
    init_scene(width, height);
}
void Pixel3D::init_window(uint32_t width, uint32_t height, const std::string &title)
{
    window_ = std::make_unique<glcpp::Window>(width, height, title);
    window_->set_factor();
    window_->set_user_pointer(this);
    init_callback();
}
void Pixel3D::init_callback()
{
    window_->set_framebuffer_size_callback(framebuffer_size_callback);
    window_->set_scroll_callback(scroll_callback);
    window_->set_mouse_button_callback(mouse_btn_callback);
    window_->set_cursor_pos_callback(mouse_callback);
}
void Pixel3D::init_ui()
{
    ui_ = std::make_unique<ui::MainLayer>();
    ui_->init(window_->get_handle());
}
void Pixel3D::init_shared_resources()
{
    shared_resources_ = std::make_shared<SharedResources>();
}
void Pixel3D::init_scene(uint32_t width, uint32_t height)
{
    scenes_.push_back(std::make_shared<MainScene>(width, height, shared_resources_));
}
void Pixel3D::loop()
{
    glfwSwapInterval(0);
    start_time_ = static_cast<float>(glfwGetTime());

    while (!window_->should_close())
    {
        update();
        process_input(window_->get_handle());

        pre_draw();
        {
            ui_->begin();
            ui_->draw_dock(fps_);
            this->draw_scene();
            auto entity = scenes_[current_scene_idx_]->get_mutable_selected_entity();
            ui_->draw_model_properties(entity);
            ui_->draw_hierarchy_layer(entity);
            ui_->draw_timeline(scenes_[current_scene_idx_].get());
            ui_->end();
        }
        post_draw();
    }
}
void Pixel3D::update()
{
    update_time();
    update_resources();
}
void Pixel3D::update_time()
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
    for (auto &scene : scenes_)
    {
        scene->set_delta_time(delta_frame_);
    }
}
void Pixel3D::update_resources()
{
    auto &shared_resources = scenes_[current_scene_idx_]->get_mutable_ref_shared_resources();
    auto entity = scenes_[current_scene_idx_]->get_mutable_selected_entity();
    auto &ui_context = ui_->get_context();
    if (ui_context.menu_context.clicked_import_model)
    {
        std::pair<bool, bool> result = shared_resources->add_model_or_animation_by_path(nullptr);
        if (result.first)
        {
            auto model = shared_resources->back_mutable_model();
            entity->set_model(model);
            entity->get_mutable_transform().set_translation(glm::vec3{0.0f, 0.0f, 0.0f}).set_rotation(glm::vec3{0.0f, 0.0f, 0.0f}).set_scale(glm::vec3{1.0f, 1.0f, 1.0f});
        }

        if (result.second)
        {
            auto animation = shared_resources->back_mutable_animation();
            entity->set_animation_component(animation);
        }
    }
    if (ui_context.menu_context.clicked_export_animation)
    {
    }
}
void Pixel3D::pre_draw()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_->get_width(), window_->get_height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Pixel3D::draw_scene()
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
void Pixel3D::post_draw()
{
    glfwSwapBuffers(window_->get_handle());
    glfwPollEvents();
}

void Pixel3D::process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::FORWARD, delta_frame_);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::BACKWARD, delta_frame_);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::LEFT, delta_frame_);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        scenes_[current_scene_idx_]->get_mutable_ref_camera()->process_keyboard(glcpp::RIGHT, delta_frame_);
}
void Pixel3D::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Pixel3D *>(glfwGetWindowUserPointer(window));

    app->window_->update_window();
}
void Pixel3D::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    auto app = reinterpret_cast<Pixel3D *>(glfwGetWindowUserPointer(window));
    if (app->is_pressed_)
    {
        app->scenes_[app->current_scene_idx_]->get_mutable_ref_camera()->process_mouse_movement((static_cast<float>(yposIn) - app->prev_mouse_.y) / 3.6f, (static_cast<float>(xposIn) - app->prev_mouse_.x) / 3.6f);
        app->prev_mouse_.x = xposIn;
        app->prev_mouse_.y = yposIn;
    }
    if (app->is_pressed_scroll_)
    {
        app->scenes_[app->current_scene_idx_]->get_mutable_ref_camera()->process_mouse_scroll_press((static_cast<float>(yposIn) - app->prev_mouse_.y), (static_cast<float>(xposIn) - app->prev_mouse_.x), app->delta_frame_);
        app->prev_mouse_.x = xposIn;
        app->prev_mouse_.y = yposIn;
    }
    app->cur_mouse_.x = xposIn;
    app->cur_mouse_.y = yposIn;
}
void Pixel3D::mouse_btn_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto app = reinterpret_cast<Pixel3D *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1)))
    {
        app->prev_mouse_.x = app->cur_mouse_.x;
        app->prev_mouse_.y = app->cur_mouse_.y;
        app->is_pressed_ = true;
    }
    else
    {
        app->is_pressed_ = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1)))
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

void Pixel3D::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto app = reinterpret_cast<Pixel3D *>(glfwGetWindowUserPointer(window));
    if (app->scenes_[app->current_scene_idx_] && app->ui_ && app->ui_->is_scene_layer_hovered("scene" + std::to_string(app->current_scene_idx_ + 1)))
        app->scenes_[app->current_scene_idx_]->get_mutable_ref_camera()->process_mouse_scroll(yoffset);
}