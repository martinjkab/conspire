#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "ecs/world.h"

class Conspire
{
public:
    void run();

private:
    void initECS();
};