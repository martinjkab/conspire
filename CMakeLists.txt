cmake_minimum_required(VERSION 3.10)

# Set minimum policy version to handle compatibility issues with dependencies
set(CMAKE_POLICY_DEFAULT_CMP0025 NEW)
if(POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()

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
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include(FetchContent)
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        v1.4.321
)
FetchContent_Declare(
    fetch_lodepng
    GIT_REPOSITORY https://github.com/lvandeve/lodepng
    GIT_TAG        master
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
FetchContent_MakeAvailable(fetch_lodepng)

# Create lodepng library target
add_library(lodepng STATIC ${fetch_lodepng_SOURCE_DIR}/lodepng.cpp)
target_include_directories(lodepng PUBLIC ${fetch_lodepng_SOURCE_DIR})

target_link_libraries(conspire PRIVATE Vulkan::Vulkan glfw glm::glm vk-bootstrap::vk-bootstrap GPUOpen::VulkanMemoryAllocator fmt::fmt lodepng)

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
    ${CMAKE_BINARY_DIR}/_deps/fetch_lodepng-src
)

# Find the glslc shader compiler from the Vulkan SDK
find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Please ensure the Vulkan SDK is installed and in your PATH.")
endif()

# --- Shader Compilation ---
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/rendering/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/src/rendering/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/src/rendering/shaders/*.comp"
)

if(GLSL_SOURCE_FILES)
    set(SPIRV_FILES "")
    foreach(GLSL_FILE ${GLSL_SOURCE_FILES})
        get_filename_component(SHADER_NAME ${GLSL_FILE} NAME_WE)
        get_filename_component(SHADER_EXT ${GLSL_FILE} EXT)
        set(SPIRV_FILE "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}${SHADER_EXT}.spv")

        # Add a command to compile each shader
        add_custom_command(
            OUTPUT ${SPIRV_FILE}
            COMMAND ${GLSLC_EXECUTABLE} -o ${SPIRV_FILE} ${GLSL_FILE}
            DEPENDS ${GLSL_FILE}
            COMMENT "Compiling ${GLSL_FILE}"
        )
        # Add the compiled shader to the list of files for our target
        list(APPEND SPIRV_FILES ${SPIRV_FILE})
    endforeach()

    # Create a target for compiling shaders that depends on the generated SPIR-V files
    add_custom_target(Shaders DEPENDS ${SPIRV_FILES})

    # Make the main executable depend on the Shaders target
    add_dependencies(conspire Shaders)

    # Copy compiled shaders to the executable directory after build
    add_custom_command(TARGET conspire POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/shaders"
                "$<TARGET_FILE_DIR:conspire>/shaders"
        COMMENT "Copying shaders to executable directory"
    )
endif()
