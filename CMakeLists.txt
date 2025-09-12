cmake_minimum_required(VERSION 3.10)

# Enable vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(conspire VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Automatically find all source and header files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Add the executable
add_executable(conspire ${SOURCES})

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
include(FetchContent)
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        v1.4.321
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)

target_link_libraries(conspire PRIVATE Vulkan::Vulkan glfw glm::glm vk-bootstrap::vk-bootstrap)

# Cross-platform settings
if(MSVC)
    target_compile_options(conspire PRIVATE /W4)
else()
    target_compile_options(conspire PRIVATE -Wall -Wextra -pedantic)
endif()

target_include_directories(conspire PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ecs
    ${CMAKE_SOURCE_DIR}/rendering
)
