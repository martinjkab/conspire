#pragma once

#include "debug_draw.h"
#include "phase.h"
#include <components/mesh.h>
#include <components/transform.h>
#include <core/window_handle.h>
#include <ecs/plugin.h>
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include <rendering/components/cameras/perspective_camera.h>
#include <rendering/components/mesh_material.h>
#include <rendering/engine.h>
#include <rendering/materials/standard_material.h>
#include <rendering/materials/standard_material_loader.h>

struct DebugDrawPlugin : public Plugin {
  void onAdd(App& app) const override {
    app.addResource(DebugDraw{})
        .addSystem(POST_UPDATE,
                   [](Resource<DebugDraw> debugDraw) { debugDraw->clear(); });
    ;
  }
};