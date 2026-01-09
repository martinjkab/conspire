#pragma once
#include <ecs/app.h>
#include <ecs/world.h>

class App;

struct Plugin {
  virtual void onAdd(App& app) const = 0;
};