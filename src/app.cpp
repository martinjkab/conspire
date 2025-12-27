#include "app.h"

#include <rendering/rendering.h>

#include "ecs/world.h"

void Conspire::run() {
  auto renderEngine = RenderEngine();
  renderEngine.init();
  renderEngine.mainLoop();
  initECS();
}

void Conspire::initECS() {}