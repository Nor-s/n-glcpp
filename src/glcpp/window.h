#ifndef GLCPP_WINDOW_WINDOW_H
#define GLCPP_WINDOW_WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
namespace glcpp
{
class Window
{
public:
	Window() = delete;
	Window(uint32_t width, uint32_t height, std::string title);
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	virtual ~Window();

	bool should_close();
	void close();
	void process_events();
	void init_glfw();
	void destroy_window();
	void wait_events();

	void set_factor();
	void set_window(uint32_t width, uint32_t height, const std::string& title);
	void set_size(uint32_t width, uint32_t height);
	void set_title(const std::string& title);
	void set_user_pointer(void* pointer);
	void set_framebuffer_size_callback(void (*fp)(GLFWwindow* window, int width, int height));
	void set_drop_callback(void (*fp)(GLFWwindow* window, int count, const char** paths));
	void set_scroll_callback(void (*fp)(GLFWwindow* window, double xoffset, double yoffset));
	void set_mouse_button_callback(void (*fp)(GLFWwindow* window, int button, int action, int modes));
	void set_cursor_pos_callback(void (*fp)(GLFWwindow* window, double xpos, double ypos));

	GLFWwindow* get_handle() const;
	std::pair<uint32_t, uint32_t> get_size() const;
	uint32_t get_width() const;
	uint32_t get_height() const;
	std::string get_title() const;
	std::pair<const char**, uint32_t> get_required_instance_extensions() const;
	std::pair<int, int> get_framebuffer_size() const;
	float get_factor();
	float get_aspect() const;
	void update_window();

private:
	GLFWwindow* handle_ = nullptr;
	float factor_;
	uint32_t width_ = 512;
	uint32_t height_ = 512;
	std::string title_ = "";
};
}	 // namespace glcpp
#endif	  // #ifndef VKCPP_MAIN_WINDOW_H