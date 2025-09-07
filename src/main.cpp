#include <iostream>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "app.h"

int main()
{
    Conspire app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
