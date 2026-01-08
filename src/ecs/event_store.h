#pragma once

#include <vector>

template <typename T>
class EventStore {
 public:
 private:
  std::vector<T> current_events;
};