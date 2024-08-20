#ifndef SRC_SCENE_MAIN_SCENE_H
#define SRC_SCENE_MAIN_SCENE_H

#include "scene.hpp"

#include <vector>
namespace anim
{
class Entity;
class Image;
}	 // namespace anim
class MainScene : public Scene
{
public:
	MainScene() = delete;
	MainScene(uint32_t width, uint32_t height, std::shared_ptr<anim::SharedResources> resources = nullptr);
	~MainScene() = default;
	void init_framebuffer(uint32_t width, uint32_t height) override;
	void pre_draw() override;
	void draw() override;
	void picking(int x, int y, bool is_only_bone) override;

private:
	void init_camera();
	void update_framebuffer();
	void draw_to_framebuffer();

	glm::vec4 background_color_{0.3f, 0.3f, 0.3f, 1.0f};

	std::shared_ptr<anim::Image> grid_framebuffer_;
};

#endif