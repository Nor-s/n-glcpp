#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace glcpp
{

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific
// input methods
enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};
const float SPEED = 100.0f;
const float SENSITIVITY = 0.03f;
const float ZOOM = 45.0f;

class Camera
{
public:
	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		   glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		   float yaw = -90.0f,
		   float pitch = 0.0f);
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
	~Camera() = default;

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	const glm::mat4& get_view() const;
	const glm::mat4& get_projection() const;
	glm::vec3 get_current_pos();

	void set_view_and_projection(float aspect);

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera
	// defined ENUM (to abstract it from windowing systems)
	void process_keyboard(CameraMovement direction, float deltaTime);

	// http://www.gisdeveloper.co.kr/?p=206
	//  TODO: arcball (trackball)
	void process_mouse_movement(float x_angle, float y_angle);

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void process_mouse_scroll(float yoffset);
	void process_mouse_scroll_press(float yoffset, float xoffset, float deltaTime);

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void update_camera_vectors();

	glm::mat4 projection_ = glm::mat4(1.0f);
	glm::mat4 view_ = glm::mat4(1.0f);
	// camera Attributes
	glm::vec3 position_;
	glm::vec3 front_;
	glm::vec3 up_;
	glm::vec3 right_;
	glm::vec3 world_up_;
	// euler Angles
	float yaw_;
	float pitch_;
	// camera options
	float movement_sensitivity_;
	float angle_sensitivity_;
	float zoom_;
	float x_angle_ = 0.0f;
	float y_angle_ = 0.0f;
};
}	 // namespace glcpp
#endif