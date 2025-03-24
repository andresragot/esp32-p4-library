# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_compile_options(-fdiagnostics-color=always -Wunused-variable -Wmissing-field-initializers -w -DDEV_DEBUG)
# add_compile_options(-fdiagnostics-color=always -Wunused-variable -Wmissing-field-initializers -w)

set(GCC_COLORS "error=01;31:warning=01;33:note=01;36:caret=01;32:locus=01:quote=01")

project(memory_test VERSION 1.0.0)