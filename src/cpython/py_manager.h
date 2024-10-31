#ifndef ANIM_CPYTHON_PY_MANAGER_H
#define ANIM_CPYTHON_PY_MANAGER_H

#include <atomic>
#include <string>
#include <thread>
#include <functional>
#include <mutex>

namespace anim
{
struct MediapipeInfo
{
	std::string video_path;
	std::string output_path;
	std::string model_info;
	float min_visibility = 0.8f;
	bool is_angle_adjustment = false;
	int model_complexity = 1;
	float min_detection_confidence = 0.8f;
	float fps = 24.0f;
	float factor = 0.0f;
	std::function<void()> callback;
};
class PyManager
{
public:
	static PyManager* get_instance();
	void get_mediapipe_animation(const MediapipeInfo& mp_info);
	void update();

private:
	PyManager();
	~PyManager();
	PyManager(const PyManager&) = delete;
	PyManager(PyManager&&) noexcept = delete;
	PyManager& operator=(const PyManager&) = delete;
	PyManager& operator=(PyManager&&) noexcept = delete;

	void work();

private:
	inline static PyManager* instance_ = nullptr;
	std::thread python_thread_;
	std::atomic_bool b_is_running_{false};
	std::atomic_bool b_is_task_pushed_{false};
	std::atomic_bool b_is_thread_work_{false};
	std::string python_path_;
	std::mutex callback_lock_;
	std::vector<std::function<void()>> callbacks_;
	MediapipeInfo mp_info_;
};
}	 // namespace anim

#endif