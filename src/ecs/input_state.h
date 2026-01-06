#pragma once

#include <bitset>

static const int keyCount = 512;

struct InputState {
  std::bitset<keyCount> pressed;
  std::bitset<keyCount> down;
  std::bitset<keyCount> released;
};