cmake_minimum_required(VERSION 3.8)

project(test)

SET(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST REQUIRED gstreamer-audio-1.0)

add_executable(test main.cpp)

target_include_directories(
    test
    PUBLIC
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(
  test
  ${GST_LIBRARIES}
  pthread
)

target_compile_options(
  test
  PUBLIC
  ${GST_CFLAGS_OTHER}
)
