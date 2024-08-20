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
#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <pybind11/embed.h>

#ifdef _DEBUG_WAS_DEFINED
#define _DEBUG 1
#endif
#include "py_manager.h"

namespace py = pybind11;
using namespace py::literals;

int main(int argc, char** argv)
{
	// const std::wstring pythonLibName = std::filesystem::path(gLinkedPythonLibraryPath).stem().wstring();
	//     const auto exe_path = std::filesystem::path(getExecutableDir());
	//     auto python_path = exe_path / "python";

	// #ifdef IS_WINDOWS
	//     const std::wstring pythonHome = python_path.wstring();
	//     auto lib_path = python_path / "Lib";
	//     auto dll_path = python_path / "DLLs";
	//     auto site_path = lib_path / "site-packages";
	//     auto app_path = exe_path / "py_module";

	//     const std::wstring pythonPath = lib_path.wstring() + L";" +
	//                                     dll_path.wstring() + L";" +
	//                                     site_path.wstring() + L";" +
	//                                     app_path.wstring() + L";" +
	//                                     python_path.wstring() + L";" +
	//                                     exe_path.wstring();
	// #else
	//     // TODO: implement for Linux
	// #endif
	//     Py_OptimizeFlag = 1;
	//     Py_SetPath(pythonPath.c_str());
	//     Py_SetPythonHome(pythonHome.c_str());
	auto py = anim::PyManager::get_instance();
	try
	{
		std::cout << "============Importing module..." << std::endl;
		// py::scoped_interpreter guard{};

		std::cout << "============Importing module..." << std::endl;
		auto mph = py::module::import("py_module.mp_helper");
		auto mpm = py::module::import("py_module.mp_manager");
		std::cout << "============Importing module..." << std::endl;

		// auto mp = py::module::import("mediapipe");
		// std::cout << "============Importing module..." << std::endl;
		// auto cap = cv2.attr("VideoCapture")("E:\\tmp\\Pixel3D\\Pixel3D\\gif\\yoga.gif");
		// std::cout << "============Importing module..." << std::endl;
		// auto mp_solution = mp.attr("solutions");
		// auto mp_pose = mp_solution.attr("pose");
		// std::cout << "============Importing module..." << std::endl;

		// auto pyqt5 = py::module::import("PyQt5");
		// std::cout << "============Impossssssssodule..." << std::endl;

		// std::cout << "1. Initializing class..." << std::endl;
		// const auto example = py::module::import("py_module.example");
		// std::cout << "2 Initializing class..." << std::endl;
		// const auto myClass = example.attr("MediapipeManager");
		// auto instance = myClass();
		// std::cout << "3 Initializing class..." << std::endl;
		// const auto pose = instance.attr("get_pose")();
		// std::cout << "4 Initializing class..." << std::endl;
		// const auto exam = py::module::import("py_module.mptest");
		// std::cout << "5 Initializing class..." << std::endl;
		// exam.attr("execc")();
		// std::cout << "6 Initializing class..." << std::endl;
		// const auto myExampleClass = example.attr("Example");
		// auto myExampleInstance = myExampleClass("Hello World"); // Calls the constructor
		// Will print in the terminal window:
		// Example constructor with msg: Hello World

		// const auto msg = myExampleInstance.attr("getMsg")(); // Calls the getMsg
		// std::cout << "Got msg back on C++ side: " << msg.cast<std::string>() << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Something went wrong: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}