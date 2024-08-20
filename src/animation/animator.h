#ifndef ANIM_ANIMATION_ANIMATOR_H
#define ANIM_ANIMATION_ANIMATOR_H

#include <glm/glm.hpp>
#include <vector>
#include <string_view>

namespace anim
{
	const unsigned int MAX_BONE_NUM = 128;

	class Model;
	class Shader;
	class Animation;
	class Entity;
	class AnimationComponent;

	class Animator
	{
	public:
		Animator();
		void update(float dt);
		void update_animation(AnimationComponent* animation, Entity* root, Shader* shader);
		void calculate_bone_transform(Entity* entity, Animation* animation, const glm::mat4& parentTransform);
		const float get_current_time() const;
		const float get_start_time() const;
		const float get_end_time() const;
		const float get_fps() const;
		const float get_direction() const;
		const bool get_is_stop() const;
		void set_current_time(float current_time);
		void set_start_time(float time);
		void set_end_time(float time);
		void set_fps(float fps);
		void set_direction(bool is_left);
		void set_is_stop(bool is_stop);

		bool mIsRootMotion{ false };

	private:
		bool is_stop_{ true };
		float current_time_{ 0.0f };
		float start_time_{ 0.0f };
		float end_time_{ 128.0f };
		float factor_{ 1.0f };
		float fps_{ 24.0f };
		float direction_{ 1.0f };
		std::vector<glm::mat4> final_bone_matrices_;
	};
}
#endif
