#pragma once

#include <ecs/component.h>
#include <rendering/components/cameras/camera.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

class PerspectiveCamera : public ComponentBase, public Camera {
 public:
  PerspectiveCamera(float fovy, float aspect, float zNear, float zFar)
      : model{glm::perspective(fovy, aspect, zNear, zFar)} {
    model[1][1] *= -1.0f;
  };

  glm::mat4 projection() const override { return model; }

 private:
  glm::mat4 model;
};