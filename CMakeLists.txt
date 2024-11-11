cmake_minimum_required(VERSION 3.14)
project(quickmedia)
set_property(CMAKE_CXX_STANDARD 17)
add_executable(quickmedia src/main.cc, src/video.cc, src/audio.cc)
