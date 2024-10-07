cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
include(CMakePrintHelpers)
project(DataMoveEngine)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)

add_subdirectory(Framework)
add_subdirectory(Data_Transmission_Frame)