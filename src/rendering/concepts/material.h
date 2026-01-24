#pragma once

#include <rendering/materials/material_context.h>
#include <rendering/utils/vk_descriptors.h>


template <typename T>
concept Material = requires(
    T& m, const T& const_m, const MaterialContext& context, VkDevice device,
    VkDescriptorSetLayout layout, DescriptorAllocator& allocator) {
  { m.init(device, layout, allocator) } -> std::same_as<void>;
  { const_m.uploadUniforms(context) } -> std::same_as<void>;
};