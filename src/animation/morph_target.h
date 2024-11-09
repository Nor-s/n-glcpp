#ifndef ANIM_ANIMATION_MORPHTARGET_H
#define ANIM_ANIMATION_MORPHTARGET_H

#include <glm/glm.hpp>
#include <glm/glm/vec3.hpp>
#include <assimp/scene.h>
#include <memory>

namespace anim
{

// UCLASS(hidecategories = Object, MinimalAPI)
// class UMorphTarget : public UObject
//{
//	GENERATED_UCLASS_BODY()
//
// public:
//	/** USkeletalMesh that this vertex animation works on. */
//	UPROPERTY(AssetRegistrySearchable)
//	TObjectPtr<class USkeletalMesh> BaseSkelMesh;
//
//	/** morph mesh vertex data for each LOD */
//	virtual const TArray<FMorphTargetLODModel>& GetMorphLODModels() const
//	{
//		return MorphLODModels;
//	}
//
//	/** morph mesh vertex data for each LOD */
//	virtual TArray<FMorphTargetLODModel>& GetMorphLODModels()
//	{
//		return MorphLODModels;
//	}
//
// protected:
//	/** morph mesh vertex data for each LOD */
//	TArray<FMorphTargetLODModel> MorphLODModels;
//
// public:
//	/** Get Morphtarget Delta array for the given input Index */
//	ENGINE_API virtual const FMorphTargetDelta* GetMorphTargetDelta(int32 LODIndex, int32& OutNumDeltas) const;
//	ENGINE_API virtual bool HasDataForLOD(int32 LODIndex) const;
//	/** return true if this morphtarget contains data for section within LOD */
//	ENGINE_API virtual bool HasDataForSection(int32 LODIndex, int32 SectionIndex) const;
//	/** return true if this morphtarget contains valid vertices */
//	ENGINE_API virtual bool HasValidData() const;
//	ENGINE_API virtual void EmptyMorphLODModels();
//
//	/** Discard CPU Buffers after render resources have been created. */
//	UE_DEPRECATED(5.0,
//				  "No longer in use, will be deleted. Whether to discard vertex data is now determined during cooking "
//				  "instead of loading.")
//	ENGINE_API virtual void DiscardVertexData();
//
//	/** Return true if this morph target uses engine built-in compression */
//	virtual bool UsesBuiltinMorphTargetCompression() const
//	{
//		return true;
//	}
//
// #if WITH_EDITOR
//	/** Populates the given morph target LOD model with the provided deltas */
//	ENGINE_API virtual void PopulateDeltas(const TArray<FMorphTargetDelta>& Deltas,
//										   const int32 LODIndex,
//										   const TArray<struct FSkelMeshSection>& Sections,
//										   const bool bCompareNormal = false,
//										   const bool bGeneratedByReductionSetting = false,
//										   const float PositionThreshold = UE_THRESH_POINTS_ARE_NEAR);
//	/** Remove empty LODModels */
//	ENGINE_API virtual void RemoveEmptyMorphTargets();
//	/** Factory function to define type of FinishBuildData needed*/
//	ENGINE_API virtual TUniquePtr<FFinishBuildMorphTargetData> CreateFinishBuildMorphTargetData() const;
// #endif	  // WITH_EDITOR
//
// public:
//	//~ UObject interface
//
//	ENGINE_API virtual void Serialize(FArchive& Ar) override;
// #if WITH_EDITORONLY_DATA
//	ENGINE_API static void DeclareCustomVersions(FArchive& Ar, const UClass* SpecificSubclass);
// #endif
//	ENGINE_API virtual void PostLoad() override;
//
//	/** UObject does not support serialization via FMemoryArchive, so manually handle separately */
//	ENGINE_API virtual void SerializeMemoryArchive(FMemoryArchive& Ar);
// };

struct MorphTargetDelta
{
	/** change in position */
	glm::vec3 position_delta;
	/** Tangent basis normal */
	glm::vec3 tangent_z_delta;
};

class MorphTarget
{
	friend std::unique_ptr<MorphTarget> CreateMorphTarget(const aiScene* scene, const aiString target_name);

public:
	MorphTarget();
	~MorphTarget();

	const std::string& get_name() const
	{
		return name_;
	}
	const std::vector<std::string>& get_mesh_names() const
	{
		return mesh_names_;
	}

private:
	std::string name_;
	std::vector<std::string> mesh_names_;
	std::vector<MorphTargetDelta> morph_target_deltas_;
};

std::unique_ptr<MorphTarget> CreateMorphTarget(const aiScene* scene, const aiString target_name);
std::vector<std::shared_ptr<MorphTarget>> CreateMorphTargets(const aiScene*);

}	 // namespace anim

#endif
