cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
include(CMakePrintHelpers)
project(DataMoveEngine)

include(../build/cmake/global.cmake)

add_subdirectory(DME_Framework)
add_subdirectory(Data_Transmission_Frame)