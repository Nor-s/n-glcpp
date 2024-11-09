#include "morph_target.h"
#include "log.h"
#include "../util/utility.h"

#include <set>

namespace anim
{
MorphTarget::MorphTarget()
{
}

MorphTarget::~MorphTarget()
{
}

// vrm4u readMorph2
// normal is not used
std::unique_ptr<MorphTarget> CreateMorphTarget(const aiScene* scene, const aiString target_name)
{
	const int RESERVE_CAPACITY_FOR_VERTICES = 2000;
	std::unique_ptr<MorphTarget> morph_target = std::make_unique<MorphTarget>();
	std::vector<MorphTargetDelta>& morph_target_deltas = morph_target->morph_target_deltas_;

	morph_target->name_ = target_name.C_Str();
	morph_target->mesh_names_.reserve(scene->mNumMeshes);
	morph_target_deltas.reserve(RESERVE_CAPACITY_FOR_VERTICES);

	for (uint32_t mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index)
	{
		const aiMesh& mesh = *(scene->mMeshes[mesh_index]);
		int32_t anim_index = -1;

		// find anim mesh
		for (uint32_t anim_mesh_index = 0; anim_mesh_index < mesh.mNumAnimMeshes; ++anim_mesh_index)
		{
			const aiAnimMesh& anim_mesh = *(mesh.mAnimMeshes[anim_mesh_index]);
			if (target_name != anim_mesh.mName)
			{
				continue;
			}
			if (anim_mesh.mNumVertices != mesh.mNumVertices)
			{
				LOG("missing vertices");
				assert(false);
				continue;
			}
			anim_index = anim_mesh_index;
			break;
		}
		if (anim_index == -1)
			continue;

		const aiAnimMesh& anim_mesh = *(mesh.mAnimMeshes[anim_index]);
		morph_target->mesh_names_.push_back(mesh.mName.C_Str());
		for (uint32_t anim_mesh_vertex_index = 0; anim_mesh_vertex_index < anim_mesh.mNumVertices;
			 ++anim_mesh_vertex_index)
		{
			MorphTargetDelta delta = {glm::vec3(0.0f), glm::vec3(0.0f)};
			if (anim_mesh.mVertices)
			{
				auto vert = anim_mesh.mVertices[anim_mesh_vertex_index] - mesh.mVertices[anim_mesh_vertex_index];
				delta.position_delta = AiVecToGlmVec(vert);
			}

			morph_target_deltas.emplace_back(delta);
		}	 // vertex loop
	}
	morph_target_deltas.shrink_to_fit();
	if (morph_target_deltas.size() == 0)
	{
		return nullptr;
	}

	return morph_target;
}

std::vector<std::shared_ptr<MorphTarget>> CreateMorphTargets(const aiScene* scene)
{
	assert(scene);

	std::set<std::string> morph_names;
	std::vector<std::shared_ptr<MorphTarget>> morph_targets;

	for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
	{
		const aiMesh& mesh = *(scene->mMeshes[m]);
		for (uint32_t a = 0; a < mesh.mNumAnimMeshes; ++a)
		{
			const aiAnimMesh& anim_mesh = *(mesh.mAnimMeshes[a]);
			std::vector<MorphTargetDelta> morph_target_deltas;
			std::string target_name = anim_mesh.mName.C_Str();

			if (morph_names.find(target_name) != morph_names.end())
			{
				continue;
			}
			morph_names.emplace(target_name);

			if (auto morph_target = CreateMorphTarget(scene, anim_mesh.mName); morph_target)
			{
				morph_targets.push_back(std::move(morph_target));
			}
		}
	}

	return morph_targets;
}

}	 // namespace anim