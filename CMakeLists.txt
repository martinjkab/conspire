cmake_minimum_required(VERSION 3.10)

# Enable vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(conspire VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the executable
add_executable(conspire src/main.cpp src/app.cpp "src/ecs/utils/remove_smart_ptr.h" "src/ecs/utils/vec_to_tuple.h" "src/rendering/components/transform.h" "src/ecs/ecs.h" "src/rendering/systems/startup_system.h" "src/rendering/engine.h" "src/rendering/engine.cpp" "src/rendering/rendering.h" "src/rendering/systems/systems.h" "src/rendering/components/components.h")

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(conspire PRIVATE Vulkan::Vulkan glfw glm::glm)

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
