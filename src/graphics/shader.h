#ifndef ANIM_GRAPHICS_SHADER_H
#define ANIM_GRAPHICS_SHADER_H
#include <glm/glm.hpp>

#include <string>

namespace anim
{

class Shader
{
public:
	// constructor generates the shader on the fly
	Shader() = default;
	virtual ~Shader() = default;
	// activate the shader
	virtual void use() = 0;
	// utility uniform functions
	virtual const unsigned int get_id() const = 0;
	virtual void set_bool(const std::string& name, bool value) const = 0;
	virtual void set_int(const std::string& name, int value) const = 0;
	virtual void set_uint(const std::string& name, int value) const = 0;
	virtual void set_float(const std::string& name, float value) const = 0;
	virtual void set_vec2(const std::string& name, const glm::vec2& value) const = 0;
	virtual void set_vec2(const std::string& name, float x, float y) const = 0;
	virtual void set_vec3(const std::string& name, const glm::vec3& value) const = 0;
	virtual void set_vec3(const std::string& name, float x, float y, float z) const = 0;
	virtual void set_vec3(const std::string& name, float xyz) const = 0;
	virtual void set_vec4(const std::string& name, const glm::vec4& value) const = 0;
	virtual void set_vec4(const std::string& name, float x, float y, float z, float w) = 0;
	virtual void set_mat2(const std::string& name, const glm::mat2& mat) const = 0;
	virtual void set_mat3(const std::string& name, const glm::mat3& mat) const = 0;
	virtual void set_mat4(const std::string& name, const glm::mat4& mat) const = 0;
};
}	 // namespace anim
#endif