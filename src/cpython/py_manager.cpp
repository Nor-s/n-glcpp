#include "py_manager.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define IS_WINDOWS
#include <Windows.h>	// Needed by GetModuleFileNameW
#else
#include <libgen.h>	   // Needed by readlink
#endif

#include <filesystem>

#include <iostream>

#ifdef _DEBUG
#define _DEBUG_WAS_DEFINED 1
#undef _DEBUG
#endif
#include <Python.h>
#ifdef _DEBUG_WAS_DEFINED
#define _DEBUG 1
#endif

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <pybind11/embed.h>
#include <pybind11/stl_bind.h>

#include "../util/log.h"

namespace py = pybind11;
using namespace py::literals;

///=============================================================================
#ifdef IS_WINDOWS
std::wstring getExecutableDir()
{
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	const auto executableDir = std::wstring(exePath);
	const auto pos = executableDir.find_last_of('\\');
	if (pos != std::string::npos)
		return executableDir.substr(0, pos);
	return L"\\";
}
#elif __APPLE__
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
std::wstring getExecutableDir()
{
	char path[PATH_MAX + 1];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		printf("executable path is %s\n", path);
	auto exe_path = std::filesystem::path(path);
	return exe_path.parent_path().wstring();
}

#else
std::wstring getExecutableDir()
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count != -1)
	{
		const auto path = std::string(dirname(result));
		return std::wstring(path.begin(), path.end());
	}
	return L"/";
}
#endif

namespace anim
{
PyManager* PyManager::get_instance()
{
	if (instance_ == nullptr)
	{
		instance_ = new PyManager();
		instance_->work();
	}
	return instance_;
}

PyManager::PyManager()
{
#ifdef IS_WINDOWS
	const auto exe_path = std::filesystem::path(getExecutableDir());
	auto python_path = exe_path / "python";
	const std::wstring pythonHome = python_path.wstring();
	auto lib_path = python_path / "Lib";
	auto dll_path = python_path / "DLLs";
	auto site_path = lib_path / "site-packages";
	auto app_path = exe_path / "mp_mocap";

	python_path_ = lib_path.wstring() + L";" + dll_path.wstring() + L";" + site_path.wstring() + L";" +
				   app_path.wstring() + L";" + python_path.wstring() + L";" + exe_path.wstring();
	// Py_SetPythonHome(pythonHome.c_str());
	//  Py_SetPythonHome(L"/Users/soongunno/githubRepo/Anim/Anim/build/bin/python/bin");
#ifndef NDEBUG
	// std::wcout << pythonPath << "\n";
#endif
#else
	// auto lib_path = python_path / "lib";
	// auto dll_path = lib_path / "python3.10";
	// auto dynload_path = lib_path / "python3.10" / "lib-dynload";
	// auto site_path = lib_path / "python3.10" / "site-packages";
	// auto app_path = exe_path / "py_module";

	// const std::wstring pythonPath = lib_path.wstring() + L";" +
	//                                 dll_path.wstring() + L";" +
	//                                 site_path.wstring() + L";" +
	//                                 app_path.wstring() + L";" +
	//                                 python_path.wstring() + L";" +
	//                                 exe_path.wstring() + L";" +
	//                                 dynload_path.wstring();
#endif
}
PyManager::~PyManager()
{
	anim::LOG("~Pymanager");
	if (instance_ != nullptr)
	{
		delete instance_;
		instance_ = nullptr;
	}
	b_is_running_ = false;
	if (python_thread_.joinable())
	{
		python_thread_.join();
	}
}

void PyManager::work()
{
	b_is_running_ = true;
	python_thread_ = std::thread(
		[]()
		{
			auto* py = PyManager::get_instance();
			Py_OptimizeFlag = 1;
			Py_SetPath(py->python_path_.c_str());
			py::initialize_interpreter();
			{
				while (py->b_is_running_)
				{
					if (py->b_is_task_pushed_)
					{
						py->b_is_task_pushed_ = false;
						py->b_is_thread_work_ = true;

						try
						{
							float factor = py->mp_info_.factor;
							const auto locals =
								py::dict("video_path"_a = py->mp_info_.video_path.c_str(),
										 "model_bindpose_info"_a = py->mp_info_.model_info.c_str(),
										 "output_path"_a = py->mp_info_.output_path.c_str(),
										 "is_angle_adjustment"_a = py->mp_info_.is_angle_adjustment,
										 "model_complexity"_a = py->mp_info_.model_complexity,
										 "min_detection_confidence"_a = py->mp_info_.min_detection_confidence,
										 "min_visibility"_a = py->mp_info_.min_visibility,
										 "custom_fps"_a = py->mp_info_.fps, "custom_factor"_a = factor);
							py::exec(R"(
				import json
				import mp_mocap

				mp_mocap.set_mediapipe_status(
					is_angle_adjustment=is_angle_adjustment,
					model_complexity=model_complexity,
					min_detection_confidence=0.7,
					min_visibility=min_visibility,
					custom_fps=custom_fps,
					custom_factor=custom_factor,
					static_image_mode=False
				)
				mp_mocap.set_redis_status(
					redis_host="localhost",
					redis_port=6379,
					redis_db=0
				)
				mp_mocap.mediapipe_to_mixamo(model_bindpose_info, video_path, output_path)
            )",
									 py::globals(), locals);
							//*mp_info.factor = locals["custom_factor"].cast<float>();
							py->b_is_thread_work_ = false;
							py->callback_lock_.lock();
							py->callbacks_.emplace_back(py->mp_info_.callback);
							py->callback_lock_.unlock();
						}
						catch (const std::exception& e)
						{
							std::cerr << e.what() << std::endl;
							py->b_is_thread_work_ = false;
						}
					}
				}
			}
			py::finalize_interpreter();
		});
}

void PyManager::get_mediapipe_animation(const MediapipeInfo& mp_info)
{
	if (b_is_thread_work_ || b_is_task_pushed_)
	{
		return;
	}
	mp_info_ = mp_info;
	b_is_task_pushed_ = true;
}

void PyManager::update()
{
	if (callbacks_.empty())
	{
		return;
	}
	callback_lock_.lock();
	std::for_each(callbacks_.begin(), callbacks_.end(), [](auto callback) { callback(); });
	callbacks_.clear();
	callback_lock_.unlock();
}

}	 // namespace anim
