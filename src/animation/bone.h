#ifndef ANIM_ANIMATION_BONE_H
#define ANIM_ANIMATION_BONE_H

#include <vector>
#include <assimp/scene.h>
#include <list>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <set>
#include <iterator>
#include <map>
#include <utility>

namespace anim
{
	struct KeyPosition
	{
		glm::vec3 position;
		float time;
		const float get_time() const
		{
			return time;
		}
	};

	struct KeyRotation
	{
		glm::quat orientation;
		float time;
		const float get_time() const
		{
			return time;
		}
	};

	struct KeyScale
	{
		glm::vec3 scale;
		float time;
		const float get_time() const
		{
			return time;
		}
	};

	class Bone
	{
	public:
		static void SetUseInterpolate(bool b_use_interpolate);

	public:
		Bone();
		Bone(const std::string& name, const aiNodeAnim* channel, const glm::mat4& inverse_binding_pose);
		void update(float animation_time, float factor);
		glm::mat4& get_local_transform(float animation_time, float factor);
		const glm::mat4& get_bindpose() const;
		const std::set<float>& get_time_set() const;
		const std::string& get_name() const;
		float get_factor();
		void get_ai_node(aiNodeAnim* channel, const aiMatrix4x4& binding_pose_transform, float factor = 1.0, bool is_interpolated = true);

		void set_name(const std::string& name);
		void set_bindpose(const glm::mat4& bindpose);
		/**
		 * TODO: floor(time) 는 assimp fbx exporter에서 내보낸 fbx 파일을 위한 것
		 * 데이터 손실 가능성 있으므로 수정해야함
		*/
		void push_position(const glm::vec3& pos, float time, bool is_floor = true);
		void push_rotation(const glm::quat& quat, float time, bool is_floor = true);
		void push_scale(const glm::vec3& scale, float time, bool is_floor = true);

		glm::vec3* get_position(float animation_time)
		{
			float time = floorf(animation_time) / factor_;
			if (positions_.find(time) == positions_.end())
			{
				return nullptr;
			}
			return &positions_[time].position;
		}
		glm::quat* get_rotation(float animation_time)
		{
			float time = floorf(animation_time) / factor_;
			if (rotations_.find(time) == rotations_.end())
			{
				return nullptr;
			}
			return &rotations_[time].orientation;
		}
		glm::vec3* get_scale(float animation_time)
		{
			float time = floorf(animation_time) / factor_;
			if (scales_.find(time) == scales_.end())
			{
				return nullptr;
			}
			return &scales_[time].scale;
		}

		void replace_or_add_keyframe(const glm::mat4& transform, float time);
		void replace_or_sub_keyframe(const glm::mat4& transform, float time);
		bool sub_keyframe(float time, bool is_animation_time = false);

	private:
		template <class T>
		const T& get_start(typename std::map<float, T>& mp, float animation_time, const T& default_value);
		template <class T>
		const T& get_end(typename std::map<float, T>& mp, float animation_time, const T& default_value);
		template <class T>
		const std::pair<const T&, const T&> get_start_end(typename std::map<float, T>& mp, float animation_time, const T& default_value);
		// template <class T>
		// int get_start_vec(typename std::vector<T> &vec, float animation_time);
		float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time);
		glm::mat4 interpolate_position(float animation_time);
		glm::mat4 interpolate_rotation(float animation_time);
		glm::mat4 interpolate_scaling(float animation_time);

		// TODO: delete key vector or map
		std::map<float, KeyPosition> positions_;
		std::map<float, KeyRotation> rotations_;
		std::map<float, KeyScale> scales_;
		std::set<float> time_set_;
		// std::vector<KeyPosition> pos_;
		// std::vector<KeyRotation> rot_;
		// std::vector<KeyScale> scale_;

		std::string name_{ "" };
		glm::mat4 local_transform_{ 1.0f };
		glm::mat4 bindpose_{ 1.0f };
		float factor_{ 1.0f };
		static bool b_use_interpolate_;
	};

	template <class T>
	const T& Bone::get_start(typename std::map<float, T>& mp, float animation_time, const T& default_value)
	{
		typename std::map<float, T>::iterator lower = mp.lower_bound(animation_time);

		if (mp.size() != 0 && lower != mp.end() && animation_time == lower->first)
		{
			return lower->second;
		}
		if (lower == mp.begin() || mp.size() == 0)
		{
			return default_value;
		}
		return std::next(lower, -1)->second;
	}
	template <class T>
	const T& Bone::get_end(typename std::map<float, T>& mp, float animation_time, const T& default_value)
	{
		if (mp.size() == 0)
		{
			return default_value;
		}
		typename std::map<float, T>::iterator upper = mp.upper_bound(animation_time);
		if (upper == mp.begin())
		{
			return upper->second;
		}
		if (std::next(upper, -1)->first == animation_time || upper == mp.end())
		{
			return std::next(upper, -1)->second;
		}
		return upper->second;
	}
	template <class T>
	const std::pair<const T&, const T&> Bone::get_start_end(typename std::map<float, T>& mp, float animation_time, const T& default_value)
	{
		if (mp.size() == 0)
		{
			auto tmp_val = default_value;
			tmp_val.time = animation_time;
			return std::make_pair(default_value, tmp_val);
		}
		typename std::map<float, T>::iterator lower = mp.lower_bound(animation_time);

		if (lower != mp.end() && animation_time == lower->first)
		{
			return { lower->second, lower->second };
		}
		if (lower == mp.begin())
		{
			return { default_value, lower->second };
		}
		if (lower != mp.end())
		{
			return { std::next(lower, -1)->second, lower->second };
		}
		lower--;
		return { lower->second, lower->second };
	}
	// template <class T>
	// int Bone::get_start_vec(typename std::vector<T> &vec, float animation_time)
	// {
	//     int size = vec.size() - 1;
	//     for (int index = 0; index < size; ++index)
	//     {
	//         if (animation_time < vec[index + 1].get_time(factor_))
	//         {
	//             return index;
	//         }
	//     }
	//     return size;
	// }
}

#endif