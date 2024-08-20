#ifndef ANIM_RESOURCES_IMPORTER_H
#define ANIM_RESOURCES_IMPORTER_H

#include <utility>
#include <memory>
#include <string>
#include <vector>

struct aiScene;

namespace anim
{
class Entity;
class Animation;
class Model;
class SharedResources;
class Importer
{
public:
	Importer();
	~Importer() = default;
	std::pair<std::shared_ptr<Model>, std::vector<std::shared_ptr<Animation>>> read_file(const char* path);
	float mScale = 100.0f;

private:
	std::shared_ptr<Model> import_model(const aiScene* scene);
	std::vector<std::shared_ptr<Animation>> import_animation(const aiScene* scene);

private:
	unsigned int assimp_flag_ = 0U;
	std::string path_;
};
}	 // namespace anim

#endif