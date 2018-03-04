cmake_minimum_required(VERSION 2.6)

project(osqoop)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(Qt5 5.4 CONFIG REQUIRED Core Widgets PrintSupport)
set(CMAKE_AUTOMOC TRUE)
find_package(LibUSB)
find_package(FFTW)

add_subdirectory(src)
add_subdirectory(datasource)
add_subdirectory(processing)
add_subdirectory(helper)
