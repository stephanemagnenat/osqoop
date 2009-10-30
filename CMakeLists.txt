cmake_minimum_required(VERSION 2.6)

project(osqoop)

set(CMAKE_MODULE_PATH ${osqoop_SOURCE_DIR} ${CMAKE_MODULE_PATH})
find_package(LibUSB)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_subdirectory(src)
add_subdirectory(datasource)
add_subdirectory(processing)
add_subdirectory(helper)