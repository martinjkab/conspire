# conspire

Conspire is a modern C++ game engine built on ECS fundamentals and a Vulkan-based renderer.

## Dependencies

Before building Conspire, ensure you have the following dependencies installed:

- **CMake**: Version 3.10 or higher. Download from [cmake.org](https://cmake.org/download/).
- **vcpkg**: A package manager for C++ libraries. Install from [github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg).
- **SDL3**: Installed via vcpkg using `vcpkg install sdl3`.
- **C++ Compiler**: A modern C++ compiler such as MSVC (on Windows), GCC, or Clang.
- **Vulkan SDK**: Required for the Vulkan-based renderer. Download from [vulkan.lunarg.com](https://vulkan.lunarg.com/sdk/home).

## Quick Start

Follow these steps to get Conspire up and running:

1. **Clone the repository**:

   ```bash
   git clone https://github.com/martinjkab/conspire.git
   cd conspire
   ```

2. **Install dependencies with vcpkg**:

   - Ensure vcpkg is installed and set up.
   - Install SDL3:
     ```bash
     vcpkg install sdl3
     ```

3. **Build the project**:

   - Create a build directory:
     ```bash
     mkdir build
     cd build
     ```
   - Configure with CMake:
     If you have the `VCPKG_ROOT` environment variable set (which points to your vcpkg installation), run:
     ```bash
     cmake ..
     ```
     Otherwise, specify the vcpkg toolchain file manually:
     ```bash
     cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
     ```
     Replace `<path-to-vcpkg>` with the actual path to your vcpkg installation.
   - Build:
     ```bash
     cmake --build . --config Release
     ```

4. **Run the executable**:
   - After building, run the generated executable:
     ```bash
     ./Release/conspire.exe  # On Windows
     ./conspire  # On Linux/Mac
     ```

For more detailed instructions or troubleshooting, refer to the project's issue tracker.
