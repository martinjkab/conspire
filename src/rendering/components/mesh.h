#pragma once

#include "ecs/ecs.h"
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

struct Mesh : ComponentBase
{
	Mesh(VkDeviceAddress &&address)
		: address{std::move(address)}
	{
	}

	friend std::ostream &operator<<(std::ostream &os, const Mesh &comp)
	{
		(void)comp;
		os << "Mesh(" << comp.address << ")";
		return os;
	}

private:
	VkDeviceAddress address;
};