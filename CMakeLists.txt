cmake_minimum_required(VERSION 2.6)

project(osqoop)

if (NOT WIN32)
    option(USE_OSS "Use the Open Sound System for audio capture (deprecated on Linux)" FALSE)
    if (USE_OSS)
        add_definitions(-DUSE_OSS)
    else()
        find_package(ALSA REQUIRED)
    endif()
endif()


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(Qt5 5.4 CONFIG REQUIRED Core Widgets PrintSupport)
set(CMAKE_AUTOMOC TRUE)
find_package(LibUSB)
find_package(FFTW)

add_subdirectory(src)
add_subdirectory(datasource)
add_subdirectory(processing)
add_subdirectory(helper)
