#pragma once
#include <ecs/event_store.h>
#include <input_state.h>
#include <mouse/cursor_position.h>
#include <mouse/mouse_motion.h>

struct InputContext {
  InputState* inputState;
  CursorPosition* cursorPosition;
  EventStore<MouseMotion>* mouseMotionEvents;
};