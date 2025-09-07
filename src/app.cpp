#include "app.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include "ecs/world.h"

const char *APP_NAME = "Conspire";
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

void Conspire::run()
{
    initWindow();
    initVulkan();
    initECS();
    mainLoop();
    cleanup();
}

void Conspire::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    this->window = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, nullptr, nullptr);
}

void Conspire::initVulkan()
{
    if (!glfwVulkanSupported())
    {
        const char *errorDesc = nullptr;
        int errorCode = glfwGetError(&errorDesc);
        if (errorCode != GLFW_NO_ERROR && errorDesc)
        {
            throw std::runtime_error(errorDesc);
        }
    }
    else
    {
        std::cout << "Vulkan is supported!" << std::endl;
    }
}

class Health : public Component
{
private:
    int hp = 0;
};

class Mana : public Component
{
private:
    int mana = 0;
};

void Conspire::initECS()
{
    auto c1 = Health{};
    auto c2 = Mana{};
    auto world = World{};
    world.addEntity(c1, c2);
}

void Conspire::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Conspire::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}