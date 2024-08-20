#include "bone.h"
#include "../util/utility.h"
#include <string>
#include <util/log.h>
#include <glm/gtx/string_cast.hpp>

namespace anim
{
void Bone::SetUseInterpolate(bool b_use_interpolate)
{
	b_use_interpolate_ = b_use_interpolate;
}
bool Bone::b_use_interpolate_{true};

Bone::Bone() = default;
Bone::Bone(const std::string& name, const aiNodeAnim* channel, const glm::mat4& inverse_binding_pose)
	: name_(name), local_transform_(1.0f)
{
	set_bindpose(glm::inverse(inverse_binding_pose));
	int num_positions = channel->mNumPositionKeys;
	int num_rotations = channel->mNumRotationKeys;
	int num_scales = channel->mNumScalingKeys;
	// push position
	for (int pos_idx = 0; pos_idx < num_positions; ++pos_idx)
	{
		aiVector3D aiPosition = channel->mPositionKeys[pos_idx].mValue;
		float time = channel->mPositionKeys[pos_idx].mTime;
		push_position(AiVecToGlmVec(aiPosition), time);
	}

	// push rotation
	for (int rot_idx = 0; rot_idx < num_rotations; ++rot_idx)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rot_idx].mValue;
		float time = channel->mRotationKeys[rot_idx].mTime;
		push_rotation(AiQuatToGlmQuat(aiOrientation), time);
	}

	// push scaling
	for (int scale_idx = 0; scale_idx < num_scales; ++scale_idx)
	{
		aiVector3D scale = channel->mScalingKeys[scale_idx].mValue;
		float time = channel->mScalingKeys[scale_idx].mTime;
		push_scale(AiVecToGlmVec(scale), time);
	}

	// unbind bind pose
	for (auto time : time_set_)
	{
		auto t = positions_.find(time);
		auto r = rotations_.find(time);
		auto s = scales_.find(time);
		glm::vec3 tt = (t != positions_.end()) ? t->second.position : glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rr = (r != rotations_.end()) ? r->second.orientation : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 ss = (s != scales_.end()) ? s->second.scale : glm::vec3(1.0f, 1.0f, 1.0f);
		glm::mat4 transformation =
			glm::translate(glm::mat4(1.0f), tt) * glm::toMat4(glm::normalize(rr)) * glm::scale(glm::mat4(1.0f), ss);
		transformation = inverse_binding_pose * transformation;

		auto [translation, rotation, scale] = DecomposeTransform(transformation);
		if (t != positions_.end())
		{
			t->second.position = translation;
		}
		if (r != rotations_.end())
		{
			r->second.orientation = rotation;
		}
		if (s != scales_.end())
		{
			s->second.scale = scale;
		}
	}
}

void Bone::update(float animation_time, float factor)
{
	factor_ = factor;
	animation_time /= factor;
	glm::mat4 translation = interpolate_position(animation_time);
	glm::mat4 rotation = interpolate_rotation(animation_time);
	glm::mat4 scale = interpolate_scaling(animation_time);
	local_transform_ = translation * rotation * scale;
}

glm::mat4& Bone::get_local_transform(float animation_time, float factor)
{
	update(animation_time, factor);
	return local_transform_;
}

const glm::mat4& Bone::get_bindpose() const
{
	return bindpose_;
}

const std::set<float>& Bone::get_time_set() const
{
	return time_set_;
}

const std::string& Bone::get_name() const
{
	return name_;
}

float Bone::get_factor()
{
	return factor_;
}
void Bone::get_ai_node(aiNodeAnim* channel,
					   const aiMatrix4x4& binding_pose_transform,
					   float factor,
					   bool is_interpolated)
{
	channel->mNodeName = aiString(name_);

	std::vector<aiVectorKey> pos_keys;
	std::vector<aiQuatKey> rot_keys;
	std::vector<aiVectorKey> scl_keys;

	std::map<float, glm::mat4> transform_map;
	transform_map[0.0f] = glm::mat4(1.0f);
	for (auto time : time_set_)
	{
		float timestamp = float(int(time * factor));
		transform_map[timestamp] = get_local_transform(timestamp, factor);	  // transformation;
	}

	if (is_interpolated)
	{
		float duration = *time_set_.rbegin();
		float anim_time = 0.0f;
		std::map<float, glm::mat4> new_transform_map;
		auto before_transform = glm::mat4(1.0f);
		for (auto& t_mp : transform_map)
		{
			for (; anim_time < t_mp.first; anim_time += 1.0f)
			{
				new_transform_map[anim_time] = get_local_transform(anim_time, factor);
			}
			before_transform = new_transform_map[t_mp.first] = t_mp.second;
			anim_time = t_mp.first + 1.0f;
		}
		transform_map = new_transform_map;
	}

	for (auto& transform : transform_map)
	{
		auto transformation = AiMatToGlmMat(binding_pose_transform) * transform.second;
		auto [t, r, s] = DecomposeTransform(transformation);

		pos_keys.push_back(aiVectorKey(transform.first, GlmVecToAiVec(t)));
		rot_keys.push_back(aiQuatKey(transform.first, GlmQuatToAiQuat(r)));
		scl_keys.push_back(aiVectorKey(transform.first, GlmVecToAiVec(s)));
	}
	channel->mNumPositionKeys = pos_keys.size();	// positions_.size();
	channel->mNumRotationKeys = rot_keys.size();	// rotations_.size();
	channel->mNumScalingKeys = scl_keys.size();		// scales_.size();

	channel->mPositionKeys = new aiVectorKey[channel->mNumPositionKeys];
	channel->mRotationKeys = new aiQuatKey[channel->mNumRotationKeys];
	channel->mScalingKeys = new aiVectorKey[channel->mNumScalingKeys];

	for (int i = 0; i < pos_keys.size(); i++)
	{
		channel->mPositionKeys[i].mValue = pos_keys[i].mValue;
		channel->mPositionKeys[i].mTime = pos_keys[i].mTime;
		channel->mRotationKeys[i].mValue = rot_keys[i].mValue;
		channel->mRotationKeys[i].mTime = rot_keys[i].mTime;
		channel->mScalingKeys[i].mValue = scl_keys[i].mValue;
		channel->mScalingKeys[i].mTime = scl_keys[i].mTime;
	}
}

void Bone::set_name(const std::string& name)
{
	name_ = name;
}
void Bone::set_bindpose(const glm::mat4& bindpose)
{
	bindpose_ = bindpose;
}
void Bone::push_position(const glm::vec3& pos, float time, bool is_floor)
{
	if (is_floor && floor(time) != time)
	{
		return;
	}
	positions_[time] = {pos, time};
	time_set_.insert(time);
}
void Bone::push_rotation(const glm::quat& quat, float time, bool is_floor)
{
	if (is_floor && floor(time) != time)
	{
		return;
	}
	rotations_[time] = {quat, time};
	time_set_.insert(time);
}
void Bone::push_scale(const glm::vec3& scale, float time, bool is_floor)
{
	if (is_floor && floor(time) != time)
	{
		return;
	}
	scales_[time] = {scale, time};
	time_set_.insert(time);
}

float Bone::get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time)
{
	float scaleFactor = 0.0f;
	float midWayLength = animation_time - last_time_stamp;
	float framesDiff = next_time_stamp - last_time_stamp;
	if (last_time_stamp == next_time_stamp || framesDiff < 0.0f)
	{
		return 1.0;
	}
	scaleFactor = midWayLength / framesDiff;
	scaleFactor = b_use_interpolate_ ? scaleFactor : 0;

	return scaleFactor;
}

glm::mat4 Bone::interpolate_position(float animation_time)
{
	const auto& [p0Index, p1Index] =
		get_start_end<KeyPosition>(positions_, animation_time, KeyPosition{glm::vec3(0.0f, 0.0f, 0.0f), 0.0f});
	float scaleFactor = get_scale_factor(p0Index.get_time(), p1Index.get_time(), animation_time);

	return glm::translate(glm::mat4(1.0f), glm::mix(p0Index.position, p1Index.position, scaleFactor));
}

glm::mat4 Bone::interpolate_rotation(float animation_time)
{
	const auto& [p0Index, p1Index] =
		get_start_end<KeyRotation>(rotations_, animation_time, KeyRotation{glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 0.0f});
	float scaleFactor = get_scale_factor(p0Index.get_time(), p1Index.get_time(), animation_time);
	glm::quat finalRotation = glm::slerp(p0Index.orientation, p1Index.orientation, (float) scaleFactor);
	return glm::toMat4(glm::normalize(finalRotation));
}

glm::mat4 Bone::interpolate_scaling(float animation_time)
{
	const auto& [p0Index, p1Index] =
		get_start_end<KeyScale>(scales_, animation_time, KeyScale{glm::vec3(1.0f, 1.0f, 1.0f), 0.0f});

	float scaleFactor = get_scale_factor(p0Index.get_time(), p1Index.get_time(), animation_time);
	return glm::scale(glm::mat4(1.0f), glm::mix(p0Index.scale, p1Index.scale, scaleFactor));
}

void Bone::replace_or_add_keyframe(const glm::mat4& transform, float time)
{
	float time_stamp = floorf(time) / factor_;
	auto [lt, lr, ls] = DecomposeTransform(get_local_transform(time, factor_));
	auto [t, r, s] = DecomposeTransform(transform);

	bool is_t_changed = !(lt.x - 1e-3 < t.x && t.x < lt.x + 1e-3 && lt.y - 1e-3 < t.y && t.y < lt.y + 1e-3 &&
						  lt.z - 1e-3 < t.z && t.z < lt.z + 1e-3);
	bool is_r_changed = !(lr.x - 1e-3 < r.x && r.x < lr.x + 1e-3 && lr.y - 1e-3 < r.y && r.y < lr.y + 1e-3 &&
						  lr.z - 1e-3 < r.z && r.z < lr.z + 1e-3);
	bool is_s_changed = !(ls.x - 1e-3 < s.x && s.x < ls.x + 1e-3 && ls.y - 1e-3 < s.y && s.y < ls.y + 1e-3 &&
						  ls.z - 1e-3 < s.z && s.z < ls.z + 1e-3);
	bool is_add = false;

	// replace
	if (positions_.find(time_stamp) != positions_.end() || is_t_changed || positions_.size() == 0)
	{
		is_add = true;
		positions_[time_stamp] = {t, time_stamp};
	}
	if (rotations_.find(time_stamp) != rotations_.end() || is_r_changed || rotations_.size() == 0)
	{
		is_add = true;
		rotations_[time_stamp] = {r, time_stamp};
	}
	if (scales_.find(time_stamp) != scales_.end() || is_s_changed || scales_.size() == 0)
	{
		is_add = true;
		scales_[time_stamp] = {s, time_stamp};
	}
	if (is_add)
	{
		time_set_.insert(time_stamp);
	}
}

void Bone::replace_or_sub_keyframe(const glm::mat4& transform, float time)
{
	float time_stamp = floorf(time) / factor_;
	auto [t, r, s] = DecomposeTransform(transform);
	auto [it_t, it_r, it_s] =
		std::tuple{positions_.find(time_stamp), rotations_.find(time_stamp), scales_.find(time_stamp)};
	sub_keyframe(time);

	auto erased_transform = get_local_transform(time, factor_);
	auto [lt, lr, ls] = DecomposeTransform(erased_transform);
	float tolerance = 1e-3;
	bool is_t_changed = !(lt.x - tolerance < t.x && t.x < lt.x + tolerance && lt.y - tolerance < t.y &&
						  t.y < lt.y + tolerance && lt.z - tolerance < t.z && t.z < lt.z + tolerance);
	bool is_r_changed = !(lr.x - tolerance < r.x && r.x < lr.x + tolerance && lr.y - tolerance < r.y &&
						  r.y < lr.y + tolerance && lr.z - tolerance < r.z && r.z < lr.z + tolerance);
	bool is_s_changed = !(ls.x - tolerance < s.x && s.x < ls.x + tolerance && ls.y - tolerance < s.y &&
						  s.y < ls.y + tolerance && ls.z - tolerance < s.z && s.z < ls.z + tolerance);
	if (is_t_changed || is_r_changed || is_s_changed || time_stamp == 0.0f)
	{
		replace_or_add_keyframe(transform, time);
	}
}

bool Bone::sub_keyframe(float time, bool is_animation_time)
{
	float time_stamp = floorf(time) / factor_;
	if (is_animation_time)
	{
		time_stamp = time;
	}
	auto [it_t, it_r, it_s] =
		std::tuple{positions_.find(time_stamp), rotations_.find(time_stamp), scales_.find(time_stamp)};
	bool is_erased = false;
	if (it_t != positions_.end())
	{
		is_erased = true;
		positions_.erase(it_t);
	}
	if (it_r != rotations_.end())
	{
		is_erased = true;
		rotations_.erase(it_r);
	}
	if (it_s != scales_.end())
	{
		is_erased = true;
		scales_.erase(it_s);
	}
	time_set_.erase(time_stamp);

	return is_erased;
}
}	 // namespace anim