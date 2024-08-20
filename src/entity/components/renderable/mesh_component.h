#ifndef ANIM_ENTITY_COMPONENT_MESH_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_MESH_COMPONENT_H

#include "../component.h"

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace anim
{
class Mesh;
class Shader;
class Entity;
struct MaterialProperties;
class MeshComponent : public ComponentBase<MeshComponent>
{
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

private:
	std::vector<std::shared_ptr<Mesh>> meshes_;
	Shader* shader_;
	Entity* entity_;
};
}	 // namespace anim

#endif