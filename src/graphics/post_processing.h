#ifndef ANIM_GRAPHICS_POST_PROCESSING_H
#define ANIM_GRAPHICS_POST_PROCESSING_H

#include <memory>
#include <glm/glm.hpp>

namespace anim
{
class Framebuffer;
class Shader;
class Image;

struct OutlineInfo
{
	glm::vec3 color{1.0f, 0.5f, 0.06f};
	float width = 1.0f;
	float threshold = 0.5f;
};

class PostProcessing
{
public:
	PostProcessing();
	~PostProcessing();
	void execuate_outline_with_depth(anim::Framebuffer* framebuffer, const OutlineInfo& outline_info = OutlineInfo());
	void set_shaders(anim::Shader* screen, anim::Shader* outline);

private:
	void init(uint32_t width, uint32_t height);

	anim::Shader* screen_shader_;
	anim::Shader* outline_shader_;
	std::shared_ptr<anim::Image> intermediate_image1x1_;
	std::shared_ptr<anim::Framebuffer> intermediate_framebuffer_;
};
}	 // namespace anim

#endif
