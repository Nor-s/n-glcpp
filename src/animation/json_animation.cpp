#include "json_animation.h"
#include "bone.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <filesystem>

namespace anim
{
	glm::vec3 JsonUtil::GetPosition(const Json::Value& bone)
	{
		if (bone == "null")
		{
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}
		return glm::vec3(bone.get("x", "0.0").asFloat(), bone.get("y", "0.0").asFloat(), bone.get("z", "0.0").asFloat());
	}
	glm::quat JsonUtil::GetRotation(const Json::Value& bone)
	{
		if (bone == "null")
		{
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}
		return glm::quat(bone.get("w", "0.0").asFloat(), bone.get("x", "0.0").asFloat(), bone.get("y", "0.0").asFloat(), bone.get("z", "0.0").asFloat());
	}
	glm::vec3 JsonUtil::GetScale(const Json::Value& bone)
	{
		if (bone == "null")
		{
			return glm::vec3(1.0f, 1.0f, 1.0f);
		}
		return glm::vec3(bone.get("x", "0.0").asFloat(), bone.get("y", "0.0").asFloat(), bone.get("z", "0.0").asFloat());
	}
	glm::mat4 JsonUtil::GetMat(const Json::Value& mat_array)
	{
		if (mat_array == "null" || static_cast<uint32_t>(mat_array.size()) != 16u)
		{
			return glm::mat4(1.0f);
		}
		glm::mat4 to;
		to[0][0] = mat_array[0 * 4 + 0].asFloat();
		to[1][0] = mat_array[1 * 4 + 0].asFloat();
		to[2][0] = mat_array[2 * 4 + 0].asFloat();
		to[3][0] = mat_array[3 * 4 + 0].asFloat();
		to[0][1] = mat_array[0 * 4 + 1].asFloat();
		to[1][1] = mat_array[1 * 4 + 1].asFloat();
		to[2][1] = mat_array[2 * 4 + 1].asFloat();
		to[3][1] = mat_array[3 * 4 + 1].asFloat();
		to[0][2] = mat_array[0 * 4 + 2].asFloat();
		to[1][2] = mat_array[1 * 4 + 2].asFloat();
		to[2][2] = mat_array[2 * 4 + 2].asFloat();
		to[3][2] = mat_array[3 * 4 + 2].asFloat();
		to[0][3] = mat_array[0 * 4 + 3].asFloat();
		to[1][3] = mat_array[1 * 4 + 3].asFloat();
		to[2][3] = mat_array[2 * 4 + 3].asFloat();
		to[3][3] = mat_array[3 * 4 + 3].asFloat();
		to = glm::transpose(to);

		return to;
	}

}

namespace anim
{
	JsonAnimation::JsonAnimation(const char* animation_path)
		: Animation(animation_path)
	{
		init();
	}
	JsonAnimation::~JsonAnimation()
	{
		name_bone_map_.clear();
	}

	void JsonAnimation::reload()
	{
#ifndef NDEBUG
		std::cout << "relaod: json" << std::endl;
#endif
		destroy();
		init();
	}
	void JsonAnimation::destroy()
	{
		name_bone_map_.clear();
	}
	// TODO: ifstream => Json::Reader
	void JsonAnimation::init()
	{
		try
		{
			std::filesystem::path path = std::filesystem::u8path((path_).c_str());
			path_ = path.filename().string();
			type_ = AnimationType::Json;
			Json::Value root;
#ifndef NDEBUG
			std::cout << "init json: " << path_ << std::endl;
#endif
			std::ifstream anim_stream(path.c_str(), std::ifstream::in | std::ifstream::binary);
			anim_stream >> root;
			if (anim_stream.is_open())
			{
				anim_stream.close();
			}
			name_ = root.get("fileName", "animation").asString();
			duration_ = root.get("duration", "0").asFloat();
			fps_ = root.get("ticksPerSecond", "24").asFloat();
			const Json::Value frames = root["frames"];
			process_frames(frames);
			process_bindpose(root["bindpose"]);
		}
		catch (std::exception& e)
		{
			type_ = AnimationType::None;
#ifndef NDEBUG
			std::cout << e.what() << std::endl;
#endif
		}
	}
	void JsonAnimation::process_frames(const Json::Value& frames)
	{
		if (frames == "null")
		{
			return;
		}
		try
		{
			uint32_t size = static_cast<uint32_t>(frames.size());
			for (uint32_t idx = 0; idx < size; idx++)
			{
				float time = frames[idx].get("time", "0").asFloat();
				const auto& bones = frames[idx]["bones"];
				if (bones != "null")
				{
					process_bones(bones, time);
				}
			}
		}
		catch (std::exception& e)
		{
#ifndef NDEBUG
			std::cout << e.what() << std::endl;
#endif
		}
	}
	void JsonAnimation::process_bones(const Json::Value& bones, float time)
	{
		try
		{
			uint32_t size = static_cast<uint32_t>(bones.size());

			for (uint32_t idx = 0; idx < size; idx++)
			{
				Bone* bone;
				std::string bone_name = bones[idx].get("name", "").asString();
				auto it = name_bone_map_.find(bone_name);
				if (it != name_bone_map_.end())
				{
					bone = it->second.get();
				}
				else
				{
					name_bone_map_[bone_name] = std::make_unique<Bone>();
					bone = name_bone_map_[bone_name].get();
					bone->set_name(bone_name);
				}
				bone->push_position(JsonUtil::GetPosition(bones[idx]["position"]), time);
				bone->push_rotation(JsonUtil::GetRotation(bones[idx]["rotation"]), time);
				bone->push_scale(JsonUtil::GetScale(bones[idx]["scale"]), time);
			}
		}
		catch (std::exception& e)
		{
#ifndef NDEBUG
			std::cout << e.what() << std::endl;
#endif
		}
	}

	void JsonAnimation::process_bindpose(const Json::Value& bindpose)
	{
		if (bindpose == "null")
		{
			return;
		}
		for (auto& it : name_bone_map_)
		{
			Bone* bone = it.second.get();
			const Json::Value& bone_bindpose = bindpose[it.first];
			if (bone_bindpose != "null")
			{
				bone->set_bindpose(JsonUtil::GetMat(bone_bindpose));
			}
		}
		for (auto const& name : bindpose.getMemberNames())
		{
			name_bindpose_map_[name] = JsonUtil::GetMat(bindpose[name]);
		}
	}
}