#ifndef ANIM_ENTITY_COMPONENT_ARMATURE_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_ARMATURE_COMPONENT_H

#include "../component.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>

namespace anim
{
	class Shader;
	class Mesh;
	class PoseComponent;
	class Entity;

	class ArmatureComponent : public ComponentBase<ArmatureComponent>
	{
	public:
		static inline bool isActivate = true;
		static inline bool isApplayLocalRotation = false;
		uint32_t selectionColor;

		static void setShape(std::unique_ptr<Mesh> shape);

		void set_pose(PoseComponent* pose);

		void set_name(const std::string& name);

		void set_bone_id(int id);

		void set_bone_offset(const glm::mat4& offset);

		void set_bind_pose(const glm::mat4& bind_pose);

		void set_model_pose(const glm::mat4& model_pose);

		PoseComponent* get_pose();

		const glm::mat4& get_bone_offset() const;

		const glm::mat4& get_bindpose() const;

		void set_local_scale(int idx, float scale);

		// http : // www.opengl-tutorial.org/kr/intermediate-tutorials/tutorial-17-quaternions/

		void set_local_rotation_with_target(int idx, const glm::vec3& target);

		void set_local_rotation(int idx, const glm::quat& quat);

		const std::string& get_name() const;

		float get_local_scale();

		int get_id();

		const glm::quat& get_local_rotation();

		const glm::mat4& get_model() const;

		void set_shader(Shader* shader);

		void set_entity(Entity* entity);

		void update();

		void insert_and_update_bone();

	private:
		PoseComponent* pose_;
		int id_{ -1 };
		std::string name_;
		std::vector<float> scale_;
		std::vector<glm::quat> rotation_{};
		glm::mat4 offset_{ 1.0f };
		glm::mat4 bind_pose_{ 1.0f };
		glm::mat4 model_{ 1.0f };
		static std::unique_ptr<Mesh> armature_;
		Entity* entity_;
		Shader* shader_;
	};
}

#endif