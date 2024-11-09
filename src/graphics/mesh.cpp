#include "mesh.h"

namespace anim
{
Mesh::Mesh(std::string_view mesh_name,
		   const std::vector<Vertex>& vertices,
		   const std::vector<unsigned int>& indices,
		   const std::vector<Texture>& textures,
		   const MaterialProperties& mat_properties)
	: mesh_name_(mesh_name)
	, vertices_(vertices)
	, indices_(indices)
	, textures_(textures)
	, mat_properties_(mat_properties)
{
}
Mesh::Mesh(std::string_view mesh_name, const std::vector<Vertex>& vertices) : mesh_name_(mesh_name), vertices_(vertices)
{
}
}	 // namespace anim