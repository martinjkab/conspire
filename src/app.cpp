#include "app.h"

#include "ecs/world.h"
#include <rendering/rendering.h>

void Conspire::run()
{
	auto renderEngine = RenderEngine();
	renderEngine.init();
	renderEngine.mainLoop();
	initECS();
}

void Conspire::initECS()
{
}