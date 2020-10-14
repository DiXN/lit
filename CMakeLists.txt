cmake_minimum_required(VERSION 3.10)

project(lit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCES main.cpp command.cpp command.h)
add_executable(main ${SOURCES})

