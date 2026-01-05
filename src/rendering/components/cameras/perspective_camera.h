#pragma once

#include <glm/glm.hpp>

#include "camera.h"

class PerspectiveCamera : public ComponentBase, public Camera {
 public:
  PerspectiveCamera(float fovy, float aspect, float zNear, float zFar)
      : model{glm::perspective(fovy, aspect, zNear, zFar)} {};

  glm::mat4 projection() const override { return model; }

 private:
  glm::mat4 model;
};