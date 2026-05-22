
#pragma once

#include <glm/glm.hpp>
#include <vector>

struct DebugLine {
  glm::vec3 a;
  glm::vec3 b;
  glm::vec4 color;
};

struct DebugLineVertex {
  glm::vec4 position;
  glm::vec4 color;
};

struct DebugDraw {
  std::vector<DebugLine> lines;

  void line(glm::vec3 a, glm::vec3 b, glm::vec4 color) {
    lines.push_back({a, b, color});
  }

  void clear() { lines.clear(); }
};
