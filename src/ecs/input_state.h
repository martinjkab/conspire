#pragma once

#include <bitset>

struct InputState {
  std::bitset<256> pressed;
  std::bitset<256> down;
  std::bitset<256> released;
};