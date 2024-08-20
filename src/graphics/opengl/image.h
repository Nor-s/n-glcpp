#ifndef ANIM_GRAPHICS_OPENGL_IMAGE_H
#define ANIM_GRAPHICS_OPENGL_IMAGE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace anim
{
class Shader;
class Image
{
public:
	Image(uint32_t width, uint32_t height, GLenum format = GL_RGB);
	~Image();
	void draw(Shader& shader);

private:
	void set_quad_VAO();
	void init_texture();

private:
	uint32_t width_;
	uint32_t height_;
	GLenum format_;
	uint32_t quad_VAO_;
	uint32_t quad_VBO_;
	uint32_t texture_;
	inline static const float quad_vertices_[24] = {
		// positions   // texCoords
		-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f, 1.0f,	-1.0f, 1.0f, 0.0f, 1.0f, 1.0f,	1.0f, 1.0f};
};
}	 // namespace anim

#endif
