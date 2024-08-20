#include "post_processing.h"
#include "opengl/image.h"
#include "opengl/framebuffer.h"
#include "shader.h"
#include <glad/glad.h>

namespace anim
{
PostProcessing::PostProcessing()
{
	intermediate_image1x1_.reset(new anim::Image(1, 1));
	init(1, 1);
}
PostProcessing::~PostProcessing()
{
	intermediate_image1x1_ = nullptr;
}
void PostProcessing::init(uint32_t width, uint32_t height)
{
	anim::FramebufferSpec spec(width, height, 1, {anim::FramebufferTextureFormat::RGB},
							   anim::FramebufferTextureFormat::Depth);
	intermediate_framebuffer_.reset(new anim::Framebuffer(spec));
}
void PostProcessing::execuate_outline_with_depth(anim::Framebuffer* framebuffer, const OutlineInfo& outline_info)
{
	auto width = framebuffer->get_width();
	auto height = framebuffer->get_height();

	if (width != intermediate_framebuffer_->get_width() || height != intermediate_framebuffer_->get_height())
	{
		init(width, height);
	}
	// only fill selected entity (Silhouette)
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xff);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);

	intermediate_framebuffer_->bind_with_depth({0.0f, 0.0f, 0.0f, 1.0f});
	{
		intermediate_framebuffer_->attach_depth24_stencil8(framebuffer);
		intermediate_image1x1_->draw(*screen_shader_);
	}
	intermediate_framebuffer_->unbind();

	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

	// outline
	outline_shader_->use();
	framebuffer->bind_without_clear();
	{
		outline_shader_->set_vec3("outlineColor", outline_info.color);
		outline_shader_->set_float("outlineWidth", outline_info.width);
		outline_shader_->set_float("outlineThreshold", outline_info.threshold);
		intermediate_framebuffer_->draw(*outline_shader_);
	}
	framebuffer->unbind();
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}
void PostProcessing::set_shaders(anim::Shader* screen, anim::Shader* outline)
{
	screen_shader_ = screen;
	outline_shader_ = outline;
}
}	 // namespace anim