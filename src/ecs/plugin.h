#pragma once
#include "world.h"
#include "app.h"

struct Plugin {
  virtual App& onAdd();
};