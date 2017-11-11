cmake_minimum_required(VERSION 3.8)
project(transistor_breach_refactored)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp src/loader.cpp src/loader.h)

include_directories(${CMAKE_SOURCE_DIR}/inc/fmod/lowlevel)
include_directories(${CMAKE_SOURCE_DIR}/inc/fmod/studio)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB LIBRARIES "${CMAKE_SOURCE_DIR}/libs/*.dylib")
message("LIBRARIES = ${LIBRARIES}")

add_executable(transistor_breach_refactored ${SOURCE_FILES})
target_link_libraries(transistor_breach_refactored ${LIBRARIES})

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})