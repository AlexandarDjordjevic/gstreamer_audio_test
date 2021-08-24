cmake_minimum_required(VERSION 3.8)

project(example)

SET(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST REQUIRED gstreamer-audio-1.0)

add_executable(Example main.cpp)

target_include_directories(
    Example
    PUBLIC
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(
  Example
  ${GST_LIBRARIES}
  pthread
)

target_compile_options(
  Example
  PUBLIC
  ${GST_CFLAGS_OTHER}
)
