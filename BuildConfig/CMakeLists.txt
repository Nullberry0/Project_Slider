cmake_minimum_required(VERSION 3.20)

project(Slideshow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)

add_executable(slideshow main.cpp)

target_link_libraries(slideshow PRIVATE SDL3::SDL3)