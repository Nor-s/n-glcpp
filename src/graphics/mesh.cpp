#include "mesh.h"

namespace anim
{
Mesh::Mesh(const std::vector<Vertex>& vertices,
		   const std::vector<unsigned int>& indices,
		   const std::vector<Texture>& textures,
		   const MaterialProperties& mat_properties)
	: vertices_(vertices), indices_(indices), textures_(textures), mat_properties_(mat_properties)
{
}
Mesh::Mesh(const std::vector<Vertex>& vertices) : vertices_(vertices)
{
}
}	 // namespace anim