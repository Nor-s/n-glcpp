#include "image.h"
#include <iostream>
#include "../shader.h"
#include <vector>

namespace anim
{

Image::Image(uint32_t width, uint32_t height, GLenum format) : width_(width), height_(height), format_(format)
{
	set_quad_VAO();
	init_texture();
}
Image::~Image()
{
	glDeleteVertexArrays(1, &quad_VAO_);
	glDeleteBuffers(1, &quad_VBO_);
	glDeleteTextures(1, &texture_);
}
void Image::draw(Shader& shader)
{
	shader.use();
	glUniform1i(glGetUniformLocation(shader.get_id(), "texture_diffuse1"), 0);
	glBindVertexArray(quad_VAO_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void Image::set_quad_VAO()
{
	glGenVertexArrays(1, &quad_VAO_);
	glGenBuffers(1, &quad_VBO_);
	glBindVertexArray(quad_VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, quad_VBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices_), &quad_vertices_, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
}

void Image::init_texture()
{
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	const size_t sizet = width_ * height_ * 4;
	std::vector<GLubyte> a(sizet, 255);

	glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, format_, GL_UNSIGNED_BYTE, a.data());
	glGenerateMipmap(GL_TEXTURE_2D);
}
}	 // namespace anim