#include "gl_shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

#include "../../util/log.h"

namespace anim::gl
{
GLShader::GLShader(const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	LOG("GLShader()");
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertex_code;
	std::string fragment_code;
	std::string geometry_code;
	std::ifstream vert_shader_file;
	std::ifstream frag_shader_file;
	std::ifstream geo_shader_file;
	// ensure ifstream objects can throw exceptions:
	vert_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	frag_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	geo_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vert_shader_file.open(vertex_path);
		frag_shader_file.open(fragment_path);
		std::stringstream vert_shader_stream, frag_shader_stream;
		// read file's buffer contents into streams
		vert_shader_stream << vert_shader_file.rdbuf();
		frag_shader_stream << frag_shader_file.rdbuf();
		// close file handlers
		vert_shader_file.close();
		frag_shader_file.close();
		// convert stream into string
		vertex_code = vert_shader_stream.str();
		fragment_code = frag_shader_stream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometry_path != nullptr)
		{
			geo_shader_file.open(geometry_path);
			std::stringstream geo_shader_stream;
			geo_shader_stream << geo_shader_file.rdbuf();
			geo_shader_file.close();
			geometry_code = geo_shader_stream.str();
		}
	}
	catch (std::ifstream::failure& e)
	{
		anim::LOG("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
	}
	const char* vert_shader_code = vertex_code.c_str();
	const char* frag_shader_code = fragment_code.c_str();

	// 2. compile shaders
	unsigned int vertex_shader, fragment_shader;
	// vertex shader
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vert_shader_code, NULL);
	glCompileShader(vertex_shader);
	check_compile_errors(vertex_shader, "VERTEX");
	// fragment Shader
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &frag_shader_code, NULL);
	glCompileShader(fragment_shader);
	check_compile_errors(fragment_shader, "FRAGMENT");
	// if geometry shader is given, compile geometry shader
	unsigned int geometry = 0;
	if (geometry_path != nullptr)
	{
		const char* gShaderCode = geometry_code.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		check_compile_errors(geometry, "GEOMETRY");
	}
	// shader Program
	id_ = glCreateProgram();
	glAttachShader(id_, vertex_shader);
	glAttachShader(id_, fragment_shader);
	if (geometry_path != nullptr)
		glAttachShader(id_, geometry);
	glLinkProgram(id_);
	check_compile_errors(id_, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	if (geometry_path != nullptr)
		glDeleteShader(geometry);
}

GLShader::~GLShader()
{
	LOG("~GLShader()");

	glDeleteProgram(id_);
}
// activate the shader
// ------------------------------------------------------------------------
void GLShader::use()
{
	glUseProgram(id_);
}
const unsigned int GLShader::get_id() const
{
	return id_;
}
// utility uniform functions
// ------------------------------------------------------------------------
void GLShader::set_bool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(id_, name.c_str()), (int) value);
}
// ------------------------------------------------------------------------
void GLShader::set_int(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}
void GLShader::set_uint(const std::string& name, int value) const
{
	glUniform1ui(glGetUniformLocation(id_, name.c_str()), value);
}
// ------------------------------------------------------------------------
void GLShader::set_float(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}
// ------------------------------------------------------------------------
void GLShader::set_vec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
}
void GLShader::set_vec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(id_, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void GLShader::set_vec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
}
void GLShader::set_vec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(id_, name.c_str()), x, y, z);
}
void GLShader::set_vec3(const std::string& name, float xyz) const
{
	glUniform3f(glGetUniformLocation(id_, name.c_str()), xyz, xyz, xyz);
}
// ------------------------------------------------------------------------
void GLShader::set_vec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
}
void GLShader::set_vec4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(id_, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void GLShader::set_mat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void GLShader::set_mat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void GLShader::set_mat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void GLShader::check_compile_errors(GLuint shader, std::string type)
{
	GLint success;
	GLchar info_log[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, info_log);
			anim::LOG("ERROR::SHADER_COMPILATION_ERROR of type: " + type + "\n-- log:" + info_log);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, info_log);
			anim::LOG("ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n-- log:" + info_log);
		}
	}
}
}	 // namespace anim::gl