cmake_minimum_required(VERSION 3.20)

project(GL2)

find_package(OpenGL REQUIRED)

add_subdirectory(src bin)