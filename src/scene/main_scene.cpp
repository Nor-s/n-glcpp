#include "main_scene.h"
#include <glcpp/camera.h>
#include <entity/entity.h>
#include <graphics/opengl/image.h>
#include <graphics/opengl/framebuffer.h>
#include <graphics/shader.h>

#include <util/log.h>
#include <graphics/shader.h>
#include <graphics/opengl/image.h>
#include <graphics/opengl/framebuffer.h>

#include "../entity/components/component.h"
#include "../entity/components/pose_component.h"

#include <filesystem>
#include <graphics/post_processing.h>

namespace fs = std::filesystem;

MainScene::MainScene(uint32_t width, uint32_t height, std::shared_ptr<anim::SharedResources> resources)
{
	anim::LOG("MainScene()");
	if (resources == nullptr)
	{
		resources_ = std::make_shared<anim::SharedResources>();
	}
	else
	{
		resources_ = std::move(resources);
	}
	init_framebuffer(width, height);
	grid_framebuffer_.reset(new anim::Image{1, 1, GL_RGBA});

	init_camera();
}

void MainScene::init_framebuffer(uint32_t width, uint32_t height)
{
	set_size(width, height);

	framebuffer_.reset(new anim::Framebuffer{anim::FramebufferSpec(
		width, height, 4, {anim::FramebufferTextureFormat::RGBA8, anim::FramebufferTextureFormat::RED_INTEGER},
		anim::FramebufferTextureFormat::Depth)});
	if (framebuffer_->error())
	{
		anim::LOG("framebuffer error");
		framebuffer_.reset(new anim::Framebuffer{anim::FramebufferSpec(
			width, height, 1, {anim::FramebufferTextureFormat::RGBA8}, anim::FramebufferTextureFormat::Depth)});
	}
}

void MainScene::init_camera()
{
	camera_ = std::make_shared<glcpp::Camera>(glm::vec3(0.0f, 100.0f, 500.0f));
}

void MainScene::pre_draw()
{
	update_framebuffer();
	camera_->set_view_and_projection(framebuffer_->get_aspect());
	resources_->set_ubo_view(camera_->get_view());
	resources_->set_ubo_projection(camera_->get_projection());

	draw_to_framebuffer();
}

void MainScene::update_framebuffer()
{
	if (framebuffer_->get_width() != width_ || framebuffer_->get_height() != height_)
	{
		init_framebuffer(width_, height_);
	}
}

void MainScene::draw_to_framebuffer()
{
	auto grid_shader = resources_->get_mutable_shader("grid");
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	framebuffer_->bind_with_depth_and_stencil(background_color_);
	{
		resources_->update();
		grid_framebuffer_->draw(*grid_shader);
	}
	framebuffer_->unbind();
	if (selected_entity_)
	{
		resources_->mPostProcessing->execuate_outline_with_depth(framebuffer_.get());
	}

#ifndef NDEBUG
	auto error = glGetError();
	anim::LOG("main scene: " + std::to_string((int) error), error == GL_NO_ERROR);
#endif
}

void MainScene::draw()
{
	auto framebuffer_shader = resources_->get_mutable_shader("framebuffer");

	framebuffer_->draw(*framebuffer_shader);
}

void MainScene::picking(int x, int y, bool is_bone_picking_mode)
{
	uint32_t pixel_i = static_cast<uint32_t>(framebuffer_->read_pixel(x, y, 1));
	uint32_t pixel_x = pixel_i >> 8;
	uint32_t pixel_y = pixel_i & 0x000000FF;

	anim::LOG(std::to_string(pixel_i) + ": " + std::to_string(pixel_x) + " " + std::to_string(pixel_y));
	int pick_id = pixel_x;
	if (pick_id == 0x00000000)
	{
		anim::LOG("- - pick: background");
		set_selected_entity(nullptr);
	}
	else
	{
		auto entity = resources_->get_entity(pick_id);
		if (entity)
		{
			anim::LOG("pick entity");
			set_selected_entity(entity->get_mutable_root());
			if (is_bone_picking_mode)
			{
				if (pixel_y == 255)
				{
					set_selected_entity(entity);
				}
				else if (selected_entity_ && selected_entity_->get_component<anim::PoseComponent>())
				{
					// mesh to armature
					auto pose = selected_entity_->get_component<anim::PoseComponent>();

					set_selected_entity(pose->find(pixel_y));
					anim::LOG("pick bone: " + selected_entity_->get_name() + " " +
							  std::to_string(selected_entity_->get_component<anim::ArmatureComponent>()->get_id()));
				}
			}
		}
	}
}