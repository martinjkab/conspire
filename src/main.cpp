#include <iostream>
#include <thread>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "app.h"
#include "rendering/components/transform.h"
#include "ecs/query.h"
#include "rendering/engine.h"

int main()
{
    Conspire app;
    World world{};

    Transform transform{};
    RenderEngine engine{};

    engine.init();

    world.addEntity(transform);
    world.addSystem([](Query<Transform> transformQuery, Resource<int> value)
                    {
                        for (auto transformTuple : transformQuery){
                            const auto [transform] = transformTuple;
                            std::cout << *value.value.get() << std::endl;
                        } });
    world.addSystem([&]()
                    { engine.mainLoop(); });
    world.addResource(5);

    try
    {
        while (!world.shouldQuit)
        {
            auto begin = std::chrono::high_resolution_clock::now();
            world.runSystems();
            auto end = std::chrono::high_resolution_clock::now();
            auto diff = begin - end;
            auto sleep = std::chrono::milliseconds(1000) - diff;
            if (sleep.count() > 0)
            {
                std::this_thread::sleep_for(sleep);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
