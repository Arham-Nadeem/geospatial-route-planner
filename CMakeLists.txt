cmake_minimum_required(VERSION 3.16)
project(geospatial_route_planner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(route_planner
    src/main.cpp
)

target_include_directories(route_planner PRIVATE include)

if(MSVC)
    target_compile_options(route_planner PRIVATE /W4)
else()
    target_compile_options(route_planner PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(route_tests
    tests/route_tests.cpp
)
target_include_directories(route_tests PRIVATE include)

if(MSVC)
    target_compile_options(route_tests PRIVATE /W4)
else()
    target_compile_options(route_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME route_tests COMMAND route_tests)
