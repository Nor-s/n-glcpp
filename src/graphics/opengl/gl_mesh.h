#ifndef ANIM_GRAPHICS_OPENGL_GL_MESH_H
#define ANIM_GRAPHICS_OPENGL_GL_MESH_H

#include "../mesh.h"
#include <memory>

namespace anim::gl
{
// TODO: Refactor Create function
std::unique_ptr<Mesh> CreateBiPyramid();
class GLMesh : public anim::Mesh
{
public:
	GLMesh(const std::vector<Vertex>& vertices,
		   const std::vector<unsigned int>& indices,
		   const std::vector<Texture>& textures,
		   const MaterialProperties& mat_properties);
	GLMesh(const std::vector<Vertex>& vertices);
	~GLMesh();
	void draw(anim::Shader& shader) override;
	void draw_outline(anim::Shader& shader) override;

private:
	void init_buffer();
	void draw();

private:
	unsigned int VAO_, VBO_, EBO_;
};
}	 // namespace anim::gl
#endif