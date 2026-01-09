#pragma once
#include <ecs/event_store.h>
#include <input/input_state.h>
#include <input/mouse/cursor_position.h>
#include <input/mouse/mouse_motion.h>

struct InputContext {
  InputState* inputState;
  CursorPosition* cursorPosition;
  EventStore<MouseMotion>* mouseMotionEvents;
};