#pragma once
#include "input_state.h"
#include <ecs/event_store.h>
#include "mouse/mouse_motion.h"

struct InputContext {
  InputState* inputState;
  EventStore<MouseMotion>* mouseMotionEvents;
};