# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--warn-common -T memory.ld -T sections.ld")
# # Remover la opción conflictiva
# string(REPLACE "--no-enable-non-contiguous-regions" "" CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})


project(p4-library)