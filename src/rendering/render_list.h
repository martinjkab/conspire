#pragma once

#include <vector>
#include <glm/glm.hpp>

struct RenderListItem {
  int meshHandle;
  glm::mat4 model;
};

class RenderList {
 public:
  std::vector<RenderListItem> items;

 private:
};