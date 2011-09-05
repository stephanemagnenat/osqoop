cmake_minimum_required(VERSION 2.6)

project(osqoop)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(Qt4 REQUIRED)
find_package(LibUSB)
find_package(FFTW)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_subdirectory(src)
add_subdirectory(datasource)
add_subdirectory(processing)
add_subdirectory(helper)
