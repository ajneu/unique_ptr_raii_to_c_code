cmake_minimum_required(VERSION 3.2)

project(resources_raii)

set(target go)
set(src mytype.c main.cpp)



set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)


add_executable(${target} ${src})
