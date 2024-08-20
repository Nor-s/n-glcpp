#include "camera.h"

namespace glcpp
{
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: front_(glm::vec3(0.0f, 0.0f, -1.0f)), movement_sensitivity_(SPEED), angle_sensitivity_(SENSITIVITY), zoom_(ZOOM)
{
	position_ = position;
	world_up_ = up;
	yaw_ = yaw;
	pitch_ = pitch;
	update_camera_vectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	: front_(glm::vec3(0.0f, 0.0f, -1.0f)), movement_sensitivity_(SPEED), angle_sensitivity_(SENSITIVITY), zoom_(ZOOM)
{
	position_ = glm::vec3(posX, posY, posZ);
	world_up_ = glm::vec3(upX, upY, upZ);
	yaw_ = yaw;
	pitch_ = pitch;
	update_camera_vectors();
}

const glm::mat4& Camera::get_view() const
{
	return view_;
}
const glm::mat4& Camera::get_projection() const
{
	return projection_;
}

glm::vec3 Camera::get_current_pos()
{
	glm::mat4 mat = glm::inverse(get_view());
	return mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void Camera::set_view_and_projection(float aspect)
{
	// https://stackoverflow.com/questions/17249429/opengl-strange-rendering-behaviour-flickering-faces
	projection_ = glm::perspective(zoom_, aspect, 1.0f, 5e3f);

	glm::mat4 mat = glm::lookAt(position_, position_ + front_, up_);
	mat = glm::rotate(mat, x_angle_, glm::vec3(1.0f, 0.0f, 0.0f));
	view_ = glm::rotate(mat, y_angle_, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::process_keyboard(CameraMovement direction, float deltaTime)
{
	float velocity = movement_sensitivity_ * deltaTime;
	if (direction == FORWARD)
		position_ += up_ * velocity;
	if (direction == BACKWARD)
		position_ -= up_ * velocity;
	if (direction == LEFT)
		position_ -= right_ * velocity;
	if (direction == RIGHT)
		position_ += right_ * velocity;
}

void Camera::process_mouse_movement(float x_angle, float y_angle)
{
	x_angle_ += x_angle * angle_sensitivity_;
	y_angle_ += y_angle * angle_sensitivity_;
}

void Camera::process_mouse_scroll(float yoffset)
{
	glm::vec3 delta = front_ * movement_sensitivity_ * 0.1f;
	if (yoffset > 0 && position_.z + delta.z >= 0.0f)
		position_ += delta;
	if (yoffset < 0 && position_.z - delta.z >= 0.0f)
		position_ -= delta;
}

void Camera::process_mouse_scroll_press(float yoffset, float xoffset, float deltaTime)
{
	float velocity = movement_sensitivity_ * deltaTime;
	position_ += up_ * velocity * yoffset;
	position_ -= right_ * velocity * xoffset;
}

void Camera::update_camera_vectors()
{
	// calculate the new Front vector
	glm::vec3 front{};
	front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front.y = sin(glm::radians(pitch_));
	front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));

	front_ = glm::normalize(front);
	right_ = glm::normalize(glm::cross(front_, world_up_));
	up_ = glm::normalize(glm::cross(right_, front_));
}
}	 // namespace glcpp