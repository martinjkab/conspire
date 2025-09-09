#pragma once

#include "ecs/ecs.h"
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

struct Transform : Component
{
	glm::vec3 position;

	Transform(const glm::vec3& position = glm::vec3{ 0 })
		: position{ position } {
	}

	friend std::ostream& operator<<(std::ostream& os, const Transform& comp)
	{
		(void)comp;
		os << "Transform(" << comp.position << ")";
		return os;
	}
};