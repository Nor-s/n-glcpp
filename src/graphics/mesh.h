#ifndef ANIM_GRAPHICS_MESH_H
#define ANIM_GRAPHICS_MESH_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

namespace anim
{
class Shader;
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	// bone indexes which will influence this vertex
	int bone_ids[MAX_BONE_INFLUENCE];
	// weights from each bone
	float weights[MAX_BONE_INFLUENCE];

	Vertex() : tex_coords({0.0f, 0.0f})
	{
		init_bone();
	}

	void init_bone()
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			bone_ids[i] = -1;
			weights[i] = 0.0f;
		}
	}
	void set_bone(int boneId, float weight)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			if (bone_ids[i] < 0)
			{
				bone_ids[i] = boneId;
				weights[i] = weight;
				return;
			}
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			if (weights[i] < weight)
			{
				bone_ids[i] = boneId;
				weights[i] = weight;
				return;
			}
		}
	}
	void set_position(const glm::vec3& vec)
	{
		position = vec;
	}
	void set_normal(const glm::vec3& vec)
	{
		normal = vec;
	}
	void set_texture_coords(const glm::vec2& vec)
	{
		tex_coords = vec;
	}
	void set_tangent(const glm::vec3& vec)
	{
		tangent = vec;
	}
	void set_bitangent(const glm::vec3& vec)
	{
		bitangent = vec;
	}
};
struct MaterialProperties
{
	glm::vec3 ambient{0.8f, 0.8f, 0.8f};
	glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
	glm::vec3 specular{0.9f, 0.9f, 0.9f};
	float shininess{1.0f};
	bool has_diffuse_texture{false};
};
struct Texture
{
	unsigned int id{0};
	std::string type{""};
	std::string path{""};
};

class Mesh
{
public:
	// Mesh::Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices,
		 const std::vector<unsigned int>& indices,
		 const std::vector<Texture>& textures,
		 const MaterialProperties& mat_properties);
	Mesh(const std::vector<Vertex>& vertices);
	// virtual ~Mesh();
	virtual ~Mesh() = default;
	virtual void draw(Shader& shader) = 0;
	virtual void draw_outline(anim::Shader& shader) = 0;
	MaterialProperties& get_mutable_mat_properties()
	{
		return mat_properties_;
	}

protected:
	std::vector<Vertex> vertices_;
	std::vector<unsigned int> indices_;
	std::vector<Texture> textures_;
	MaterialProperties mat_properties_;
};
}	 // namespace anim

#endif