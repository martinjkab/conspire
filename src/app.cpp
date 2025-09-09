#include "app.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include "ecs/world.h"
#include <rendering/rendering.h>

void Conspire::run()
{
	auto renderEngine = RenderEngine();
	renderEngine.init();
	while (true){
		renderEngine.mainLoop();
	}
	initECS();
}

void Conspire::initECS()
{
}