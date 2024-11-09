#ifndef ANIM_RESOURCES_IMPORTER_H
#define ANIM_RESOURCES_IMPORTER_H

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <tuple>

struct aiScene;

namespace anim
{
class Entity;
class Animation;
class Model;
class SharedResources;
class MorphTarget;

using AnimationVector = std::vector<std::shared_ptr<Animation>>;
using MorphTargetVector = std::vector<std::shared_ptr<MorphTarget>>;
class Importer
{
public:
	Importer();
	~Importer() = default;
	std::tuple<std::shared_ptr<Model>, AnimationVector, MorphTargetVector> read_file(const char* path);

private:
	std::shared_ptr<Model> import_model(const aiScene* scene);
	AnimationVector import_animation(const aiScene* scene);
	MorphTargetVector import_morphtarget(const aiScene* scene);

private:
	unsigned int assimp_flag_ = 0U;
	std::string path_;
};
}	 // namespace anim

#endif