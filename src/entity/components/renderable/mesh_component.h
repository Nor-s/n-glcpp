#ifndef ANIM_ENTITY_COMPONENT_MESH_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_MESH_COMPONENT_H

#include "../component.h"

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <ranges>
#include <algorithm>
#include "mesh.h"

namespace anim
{
class Shader;
class Entity;
class SharedResources;
struct MaterialProperties;

class MeshComponent : public ComponentBase<MeshComponent>
{
	friend class SharedResources;

public:
	static inline bool isActivate = true;
	static inline bool isWireframe = false;
	bool isDynamic{false};
	uint32_t selectionColor;

	~MeshComponent() = default;
	void update() override;

	void set_meshes(const std::vector<std::shared_ptr<Mesh>>& meshes);
	void set_shader(Shader* shader);
	void set_entity(Entity* entity);

	std::vector<MaterialProperties*> get_mutable_mat();

	std::vector<std::string> get_meshes_name() const
	{
		std::vector<std::string> meshes_names;
		/*	std::for_each(meshes_.begin(), meshes_.end(),
						  [&meshes_names](const std::shared_ptr<Mesh>& mesh)
						  { meshes_names.push_back(mesh->get_mesh_name()); });*/
		return meshes_names;
	}

protected:
	std::shared_ptr<Mesh> get_mesh(const std::string& name) const
	{
		auto it = std::ranges::find_if(
			meshes_, [&name](const std::shared_ptr<Mesh>& mesh) { return mesh->get_mesh_name() == name; });
		return it != meshes_.end() ? *it : nullptr;
	}

private:
	std::vector<std::shared_ptr<Mesh>> meshes_;
	Shader* shader_;
	Entity* entity_;
};
}	 // namespace anim

#endif