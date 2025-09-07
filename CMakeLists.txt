cmake_minimum_required(VERSION 3.10)

# Enable vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Set the project name and version
project(conspire VERSION 1.0)

# Add the executable
add_executable(conspire src/main.cpp)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(conspire PRIVATE SDL3::SDL3)

# Cross-platform settings
if(MSVC)
    target_compile_options(conspire PRIVATE /W4)
else()
    target_compile_options(conspire PRIVATE -Wall -Wextra -pedantic)
endif()
