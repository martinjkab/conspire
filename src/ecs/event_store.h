#pragma once

#include <vector>

template <typename T>
class EventStore {
 public:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  void addEvent(T&& event) { current_events.push_back(event); }
  void update() { current_events.clear(); }

  iterator begin() { return current_events.begin(); }
  iterator end() { return current_events.end(); }

  const_iterator begin() const { return current_events.begin(); }
  const_iterator end() const { return current_events.end(); }

  const_iterator cbegin() const { return current_events.cbegin(); }
  const_iterator cend() const { return current_events.cend(); }

 private:
  std::vector<T> current_events;
};