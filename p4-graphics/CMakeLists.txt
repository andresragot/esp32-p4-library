# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

add_compile_options(-w -DDEBUG)
# add_compile_options(-w)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(p4-graphics)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)