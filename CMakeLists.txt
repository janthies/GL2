cmake_minimum_required(VERSION 3.20)

project(GL2)

find_package(OpenGL REQUIRED)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)


add_subdirectory(src bin)