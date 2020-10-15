cmake_minimum_required(VERSION 3.10)

project(lit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(lit)
target_sources(lit
  PRIVATE
    main.cpp
    command.cpp
)
target_include_directories(lit PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")

