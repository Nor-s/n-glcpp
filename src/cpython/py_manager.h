#ifndef ANIM_CPYTHON_PY_MANAGER_H
#define ANIM_CPYTHON_PY_MANAGER_H

namespace anim
{
struct MediapipeInfo
{
	const char* video_path;
	const char* output_path;
	const char* model_info;
	float min_visibility = 0.8f;
	bool is_angle_adjustment = false;
	int model_complexity = 1;
	float min_detection_confidence = 0.8f;
	float fps = 24.0f;
	float* factor = nullptr;
};
class PyManager
{
public:
	static PyManager* get_instance();
	void get_mediapipe_animation(const MediapipeInfo& mp_info);

private:
	PyManager();
	~PyManager();
	PyManager(const PyManager&) = delete;
	PyManager(PyManager&&) noexcept = delete;
	PyManager& operator=(const PyManager&) = delete;
	PyManager& operator=(PyManager&&) noexcept = delete;

private:
	inline static PyManager* instance_ = nullptr;
};
}	 // namespace anim

#endif