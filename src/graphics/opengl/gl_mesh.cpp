
#include "gl_mesh.h"
#include "../shader.h"

#include <glad/glad.h>
using namespace anim;

namespace anim::gl
{
std::unique_ptr<Mesh> CreateBiPyramid()
{
	const float position[24 * 3] = {// positions
									// front bottom
									-0.10f, 0.10f, 0.10f, 0.0f, 0.0f, 0.0f, 0.10f, 0.10f, 0.10f,
									// right
									0.10f, 0.10f, 0.10f, 0.0f, 0.0f, 0.0f, 0.10f, 0.10f, -0.10f,
									// left
									-0.10f, 0.10f, -0.10f, 0.0f, 0.0f, 0.0f, -0.10f, 0.10f, 0.10f,
									// back
									0.10f, 0.10f, -0.10f, 0.0f, 0.0f, 0.0f, -0.10f, 0.10f, -0.10f,

									// front top

									0.10f, 0.10f, 0.10f, 0.0f, 1.0f, 0.0f, -0.10f, 0.10f, 0.10f,
									// right
									0.10f, 0.10f, -0.10f, 0.0f, 1.0f, 0.0f, 0.10f, 0.10f, 0.10f,
									// left
									-0.10f, 0.10f, 0.10f, 0.0f, 1.0f, 0.0f, -0.10f, 0.10f, -0.10f,
									// back
									-0.10f, 0.10f, -0.10f, 0.0f, 1.0f, 0.0f, 0.10f, 0.10f, -0.10f};
	float normal[72]{0.0f};
	for (int i = 0; i < 72; i += 9)
	{
		glm::vec3 a{position[i], position[i + 1], position[i + 2]};
		glm::vec3 b{position[i + 3], position[i + 4], position[i + 5]};
		glm::vec3 c{position[i + 6], position[i + 7], position[i + 8]};
		glm::vec3 normal_v = glm::normalize(glm::cross(b - a, c - a));
		normal[i] = normal_v.x;
		normal[i + 1] = normal_v.y;
		normal[i + 2] = normal_v.z;
		normal[i + 3] = normal_v.x;
		normal[i + 4] = normal_v.y;
		normal[i + 5] = normal_v.z;
		normal[i + 6] = normal_v.x;
		normal[i + 7] = normal_v.y;
		normal[i + 8] = normal_v.z;
	}
	std::vector<Vertex> vertices;
	for (int i = 0; i < 72; i += 3)
	{
		Vertex vert{};
		vert.set_position({position[i], position[i + 1], position[i + 2]});
		vert.set_normal({normal[i], normal[i + 1], normal[i + 2]});
		vertices.push_back(vert);
	}

	return std::make_unique<GLMesh>(vertices);
}

GLMesh::GLMesh(const std::vector<Vertex>& vertices,
			   const std::vector<unsigned int>& indices,
			   const std::vector<Texture>& textures,
			   const MaterialProperties& mat_properties)
	: Mesh(vertices, indices, textures, mat_properties)
{
	init_buffer();
}
GLMesh::GLMesh(const std::vector<Vertex>& vertices) : Mesh(vertices)
{
	init_buffer();
}

// TODO: delete buffer
GLMesh::~GLMesh()
{
}
void GLMesh::draw(anim::Shader& shader)
{
	shader.use();
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	shader.set_vec3("material.ambient", mat_properties_.ambient);
	shader.set_vec3("material.diffuse", mat_properties_.diffuse);
	shader.set_vec3("material.specular", mat_properties_.specular);
	shader.set_float("material.shininess", mat_properties_.shininess);
	shader.set_bool("material.has_diffuse_texture", mat_properties_.has_diffuse_texture);
	shader.set_vec3("dir_lights[0].direction", 0.0f, -1.0f, -1.0f);
	shader.set_vec3("dir_lights[0].ambient", 0.5f);
	shader.set_vec3("dir_lights[0].diffuse", 0.8f);
	shader.set_vec3("dir_lights[0].specular", 0.6f);
	for (unsigned int i = 0; i < textures_.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);	 // active proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures_[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);	  // transfer unsigned int to string
		else if (name == "texture_normal")
			number = std::to_string(normalNr++);	// transfer unsigned int to string
		else if (name == "texture_height")
			number = std::to_string(heightNr++);	// transfer unsigned int to string

		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.get_id(), (name + number).c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures_[i].id);
	}
	// draw mesh
	draw();

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}
void GLMesh::draw_outline(anim::Shader& shader)
{
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_REPLACE, GL_REPLACE);
	glStencilMask(0xFF);

	draw(shader);

	glDisable(GL_STENCIL_TEST);
}
void GLMesh::draw()
{
	// draw mesh
	glBindVertexArray(VAO_);
	if (indices_.size() > 0)
	{
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices_.size()), GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
	}
	glBindVertexArray(0);
}
void GLMesh::init_buffer()
{
	glGenVertexArrays(1, &VAO_);
	glGenBuffers(1, &VBO_);

	glBindVertexArray(VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);

	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);
	if (indices_.size() > 0)
	{
		glGenBuffers(1, &EBO_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	// vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tex_coords));

	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, bitangent));
	// ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*) offsetof(Vertex, bone_ids));
	// weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, weights));
	glBindVertexArray(0);
}
}	 // namespace anim::gl