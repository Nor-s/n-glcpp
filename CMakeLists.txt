cmake_minimum_required(VERSION 3.21)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

message("Using CMake version ${CMAKE_VERSION}")

# for CCACHE
if(APPLE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  endif()
endif()

project(
  Anim
  LANGUAGES CXX
  VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# for ninja(unix)
if (UNIX AND NOT APPLE)
    set(CMAKE_MAKE_PROGRAM /usr/bin/ninja CACHE FILEPATH "")
endif()

message("CMAKE_RUNTIME_OUTPUT_DIRECTORY : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")

#  for apple ccache
if(APPLE)
  get_property(RULE_LAUNCH_COMPILE GLOBAL PROPERTY RULE_LAUNCH_COMPILE)
  if(RULE_LAUNCH_COMPILE AND CMAKE_GENERATOR STREQUAL "Xcode")
      # Set up wrapper scripts
      configure_file(launch-c.in launch-c)
      configure_file(launch-cxx.in launch-cxx)
      execute_process(COMMAND chmod a+rx
                               "${CMAKE_BINARY_DIR}/launch-c"
                               "${CMAKE_BINARY_DIR}/launch-cxx"
      )
      # Set Xcode project attributes to route compilation and linking
      # through our scripts
      set(CMAKE_XCODE_ATTRIBUTE_CC         "${CMAKE_BINARY_DIR}/launch-c")
      set(CMAKE_XCODE_ATTRIBUTE_CXX        "${CMAKE_BINARY_DIR}/launch-cxx")
      set(CMAKE_XCODE_ATTRIBUTE_LD         "${CMAKE_BINARY_DIR}/launch-c")
      set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS "${CMAKE_BINARY_DIR}/launch-cxx")
  endif()
endif()


# for output directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(COPY resources DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY imgui.ini DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY LICENSE DESTINATION ${CMAKE_BINARY_DIR}/bin)

message("==========================================================")
message("Build python")
if (APPLE)
  execute_process(COMMAND sh "scripts/build_python_osx.sh" WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}) 
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/bin/python/bin/python)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/python DESTINATION ${CMAKE_BINARY_DIR}/bin)
  endif()
else()
  execute_process(COMMAND cmd /C ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_python.bat
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/bin/python/python.exe)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/python DESTINATION ${CMAKE_BINARY_DIR}/bin)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/python/python310.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/python/python3.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
  endif()
endif()
file(COPY mp_mocap DESTINATION ${CMAKE_BINARY_DIR}/bin/python/Lib/site-packages)
message("==========================================================")

message("Compiler")
message(STATUS " - ID       \t: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " - Version  \t: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " - Path     \t: ${CMAKE_CXX_COMPILER}")

# for external option
option(JSONCPP_WITH_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(ASSIMP_BUILD_ZLIB ON)
option(ASSIMP_BUILD_TESTS OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
if(WIN32)
  option(ASSIMP_RAPIDJSON_NO_MEMBER_ITERATOR OFF)
  set(ASSIMP_RAPIDJSON_NO_MEMBER_ITERATOR OFF)
endif()
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

# add external libraries
message("GLAD")
add_subdirectory(external/glad/)
message("STB")
add_library(STB_IMAGE "external/stb/stb_image.cpp")


if(WIN32)
  message("GLFW")
  add_subdirectory(external/glfw/)
  message("IMGUI")
  add_subdirectory(external/imgui/)
  message("ASSIMP")
  add_subdirectory(external/assimp/)
else()
  add_subdirectory(external/imgui/)
  add_subdirectory(external/assimp/)
  add_subdirectory(external/glfw/)
endif()

message("jsoncpp")
add_subdirectory(external/jsoncpp)
message("GLM")
add_subdirectory(external/glm/)

message("Python" ${CMAKE_BINARY_DIR}/bin/python/)
set(Python3_ROOT_DIR ${CMAKE_BINARY_DIR}/bin/python)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
find_package(Python3 3.10 EXACT COMPONENTS Interpreter Development REQUIRED)
GET_TARGET_PROPERTY(\_lib Python3::Python IMPORTED_IMPLIB)
MESSAGE(STATUS "Python3::Python ${\_lib}")
GET_TARGET_PROPERTY(\_lib Python3::Python IMPORTED_IMPLIB_DEBUG)
MESSAGE(STATUS "Python3::Python (Debug) ${\_lib}")
GET_TARGET_PROPERTY(\_lib Python3::Python IMPORTED_IMPLIB_RELEASE)
MESSAGE(STATUS "Python3::Python (Release) ${\_lib}")

message("pybind11")
add_subdirectory(external/pybind11)
message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS} ${pybind11_DEFINITIONS} ${pybind11_LIBRARIES}")
message("==========================================================")


add_subdirectory(src)
add_dependencies (imgui glfw)
add_dependencies(STB_IMAGE imgui)
add_dependencies(glcpp assimp)
