#pragma once
#include "ecs/world.h"
#include "ecs/app.h"

class App;

struct Plugin {
  virtual void onAdd(App& app) const = 0;
};