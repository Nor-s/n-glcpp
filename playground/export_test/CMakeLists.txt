cmake_minimum_required(VERSION 3.21)

project(
  TestProject
  LANGUAGES CXX
  VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

add_executable(export_test  ./main.cpp )
target_link_libraries(export_test PUBLIC assimp::assimp)
