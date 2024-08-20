#include "animator.h"

#include <glm/gtx/matrix_decompose.hpp>
#include "entity.h"
#include "bone.h"
#include "../util/utility.h"
#include "shader.h"
#include "../entity/components/animation_component.h"
#include "../entity/components/renderable/armature_component.h"
#include "animation.h"
#include "json_animation.h"
#include "assimp_animation.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

namespace anim
{

	Animator::Animator()
		: is_stop_(true)
	{
		final_bone_matrices_.reserve(MAX_BONE_NUM);
		for (unsigned int i = 0U; i < MAX_BONE_NUM; i++)
			final_bone_matrices_.push_back(glm::mat4(1.0f));
	}

	void Animator::update(float dt)
	{
		if (!is_stop_)
		{
			current_time_ += fps_ * dt * direction_;
			current_time_ = fmax(start_time_, fmod(end_time_ + current_time_, end_time_));
		}
		else
		{
			current_time_ = floor(current_time_);
		}
	}

	void Animator::update_animation(AnimationComponent* animation_component, Entity* root, Shader* shader)
	{
		assert(animation_component && root && shader);

		factor_ = animation_component->get_ticks_per_second_factor();
		calculate_bone_transform(root, animation_component->get_mutable_animation(), glm::mat4(1.0f));
		shader->use();
		for (int i = 0; i < MAX_BONE_NUM; ++i)
		{
			shader->set_mat4("finalBonesMatrices[" + std::to_string(i) + "]", final_bone_matrices_[i]);
		}
	}

	void Animator::calculate_bone_transform(Entity* entity, Animation* animation, const glm::mat4& parentTransform)
	{
		const std::string& node_name = entity->get_name();
		auto armature = entity->get_component<ArmatureComponent>();
		glm::mat4 global_transformation = parentTransform;

		// 바인딩 포즈
		global_transformation *= armature->get_bindpose();

		// 애니메이션
		bool bIsRemoveTranslation = (entity->get_mutable_parent()->get_component<ArmatureComponent>() == nullptr && mIsRootMotion);
		const auto local = animation->get_bone_local_transform(node_name, current_time_, factor_, bIsRemoveTranslation);
		entity->set_local(local);
		global_transformation *= local;

		// FK
		const int id = armature->get_id();
		if (id < MAX_BONE_NUM)
		{
			auto& offset = armature->get_bone_offset();
			// 역바인딩변환 행렬과 변환행렬을 곱해줌 (본공간 => 로컬공간)
			final_bone_matrices_[id] = global_transformation * offset;
		}

		const auto& children = entity->get_mutable_children();
		const size_t size = children.size();
		armature->set_model_pose(global_transformation);
		for (size_t i = 0; i < size; i++)
		{
			calculate_bone_transform(children[i].get(), animation, global_transformation);
		}
	}
	const float Animator::get_current_time() const
	{
		return current_time_;
	}
	const float Animator::get_start_time() const
	{
		return start_time_;
	}
	const float Animator::get_end_time() const
	{
		return end_time_;
	}
	const float Animator::get_fps() const
	{
		return fps_;
	}
	const float Animator::get_direction() const
	{
		return direction_;
	}
	const bool Animator::get_is_stop() const
	{
		return is_stop_;
	}
	void Animator::set_current_time(float current_time)
	{
		current_time_ = current_time;
	}
	void Animator::set_start_time(float time)
	{
		start_time_ = time;
	}
	void Animator::set_end_time(float time)
	{
		end_time_ = time;
	}
	void Animator::set_fps(float fps)
	{
		fps_ = fps;
	}
	void Animator::set_direction(bool is_left)
	{
		direction_ = 1.0f;
		if (is_left)
		{
			direction_ = -1.0f;
		}
	}
	void Animator::set_is_stop(bool is_stop)
	{
		is_stop_ = is_stop;
	}

}