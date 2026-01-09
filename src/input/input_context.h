#pragma once
#include "input_state.h"
#include <ecs/event_store.h>
#include "mouse/mouse_motion.h"
#include "mouse/cursor_position.h"

struct InputContext {
  InputState* inputState;
  CursorPosition* cursorPosition;
  EventStore<MouseMotion>* mouseMotionEvents;
};